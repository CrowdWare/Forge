/*
#############################################################################
# Copyright (C) 2026 CrowdWare
#
# This file is part of Forge.
#
# SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-CrowdWare-Commercial
#
# Forge is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Forge is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Forge. If not, see <https://www.gnu.org/licenses/>.
#
# Commercial licensing is available from CrowdWare for proprietary use.
#############################################################################
*/

// sms_runner_integration_tests.cpp
//
// Integration tests for the ForgeRunner.Native SMS bridge patterns.
// Exercises SMS script execution with mock UI callbacks and a dispatch depth
// guard, reproducing the exact scenarios that cause stack overflows at runtime
// (e.g. the setKeyframe cycle in ForgePoser).
//
// No Godot dependency - links directly against sms_native + forge_sms_error_policy.

#include "../src/forge_sms_error_policy.h"
#include "sml_document.h"
#include "sms_native.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace {

static const std::string kMantra = "Tu keinem Lebewesen Leid an.\n";

// ==========================================================================
// Assertion helpers
// ==========================================================================

void assert_true(bool cond, const std::string& msg) {
    if (!cond) throw std::runtime_error(msg);
}

void assert_false(bool cond, const std::string& msg) {
    if (cond) throw std::runtime_error(msg);
}

void assert_contains(const std::string& haystack, const std::string& needle,
                     const std::string& ctx) {
    if (haystack.find(needle) == std::string::npos)
        throw std::runtime_error(ctx + ": expected \"" + needle
                                 + "\" in \"" + haystack + "\"");
}

// ==========================================================================
// Minimal SmsSession RAII
// ==========================================================================

struct SmsSession {
    std::int64_t id = -1;

    bool load(const std::string& source, std::string& out_err) {
        const std::string full = kMantra + source;
        char err[512] = {};
        if (sms_native_session_create(&id, err, static_cast<int>(sizeof(err))) != 0
                || id < 0) {
            out_err = err;
            return false;
        }
        if (sms_native_session_load(id, full.c_str(), err,
                                    static_cast<int>(sizeof(err))) != 0) {
            out_err = err;
            dispose();
            return false;
        }
        return true;
    }

    // Returns the error string; empty = success.
    std::string invoke(const std::string& target, const std::string& event,
                       const std::string& args = "[]") {
        if (id < 0) return "session not loaded";
        std::int64_t result = 0;
        char err[512] = {};
        sms_native_session_invoke(id, target.c_str(), event.c_str(),
                                  args.c_str(), &result, err,
                                  static_cast<int>(sizeof(err)));
        return std::string(err);
    }

    void dispose() {
        if (id >= 0) {
            sms_native_session_dispose(id, nullptr, 0);
            id = -1;
        }
    }

    ~SmsSession() { dispose(); }
};

// ==========================================================================
// Mock UI callback layer
// ==========================================================================

struct MockUiState {
    std::unordered_map<std::string, std::string> props;
    std::vector<std::string>                     invoke_log;
    // Optional hook: called from mock_ui_invoke (id, method, args, out, out_cap).
    std::function<int(const char*, const char*, const char*, char*, int)> invoke_hook;
};

static MockUiState* g_mock = nullptr;

static int mock_ui_get(const char* id, const char* prop,
                       char* out, int cap, char*, int) {
    const std::string prop_str = prop ? prop : "";
    // ui.getObject() probes __exists - always return truthy so __ui_ref is created.
    if (prop_str == "__exists") {
        if (out && cap > 0) std::snprintf(out, static_cast<std::size_t>(cap), "1");
        return 0;
    }
    const std::string key = std::string(id ? id : "") + "." + prop_str;
    std::string val = "null";
    if (g_mock) {
        auto it = g_mock->props.find(key);
        if (it != g_mock->props.end()) val = it->second;
    }
    if (out && cap > 0) std::snprintf(out, static_cast<std::size_t>(cap), "%s", val.c_str());
    return 0;
}

static int mock_ui_set(const char* id, const char* prop, const char* val, char*, int) {
    if (g_mock)
        g_mock->props[std::string(id ? id : "") + "." + (prop ? prop : "")] = val ? val : "null";
    return 0;
}

static int mock_ui_invoke(const char* id, const char* method, const char* args,
                          char* out, int cap, char*, int) {
    if (g_mock) {
        g_mock->invoke_log.push_back(
            std::string(id ? id : "") + "." + (method ? method : ""));
        if (g_mock->invoke_hook)
            return g_mock->invoke_hook(id, method, args, out, cap);
    }
    if (out && cap > 0) out[0] = '\0';
    return 0;
}

/// RAII: installs mock callbacks on construction, restores null on destruction.
struct ScopedMockUi {
    MockUiState state;

    ScopedMockUi() {
        g_mock = &state;
        sms_native_set_ui_callbacks(mock_ui_get, mock_ui_set, mock_ui_invoke,
                                    nullptr, 0);
    }
    ~ScopedMockUi() {
        sms_native_set_ui_callbacks(nullptr, nullptr, nullptr, nullptr, 0);
        g_mock = nullptr;
    }
};

// ==========================================================================
// SML-aware mock: parses real SML to drive ui.getObject() and initial values
//
// This is the pattern that mirrors how ForgeRunner.Native actually works:
//   SML declares elements with id="..." attributes
//   → SmlUiBuilder registers them in SmsBridge::id_map()
//   → SMS scripts call ui.getObject("id") to get a __ui_ref handle
//   → Property reads/writes route through sms_ui_get / sms_ui_set
//
// Here we replace the Godot UI tree with a parsed SML document so tests
// can be written in terms of SML + SMS without any Godot dependency.
// ==========================================================================

static std::unordered_set<std::string>*          g_sml_known_ids     = nullptr;
static std::unordered_map<std::string, std::string>* g_sml_init_props = nullptr;

// Variant of mock_ui_get that checks actual SML-declared IDs.
static int sml_ui_get(const char* id, const char* prop,
                      char* out, int cap, char*, int) {
    const std::string prop_str = prop ? prop : "";
    if (prop_str == "__exists") {
        const bool exists = g_sml_known_ids
                            && g_sml_known_ids->count(id ? id : "") > 0;
        if (out && cap > 0) std::snprintf(out, static_cast<std::size_t>(cap),
                                          "%s", exists ? "1" : "0");
        return 0;
    }
    // SML-declared initial attribute value (e.g. value="42" on a slider).
    const std::string key = std::string(id ? id : "") + "." + prop_str;
    if (g_sml_init_props) {
        auto it = g_sml_init_props->find(key);
        if (it != g_sml_init_props->end()) {
            if (out && cap > 0)
                std::snprintf(out, static_cast<std::size_t>(cap),
                              "%s", it->second.c_str());
            return 0;
        }
    }
    // Fall back to runtime-set props (written by SMS via mock_ui_set).
    return mock_ui_get(id, prop, out, cap, nullptr, 0);
}

/// RAII mock that parses an SML document and wires up ID-aware callbacks.
/// ui.getObject("id") succeeds only for elements declared in the SML;
/// SML attribute values (text="…", value="42") are readable as initial props.
struct SmlMockUi {
    MockUiState state;
    std::unordered_set<std::string>          known_ids;
    std::unordered_map<std::string, std::string> init_props;

    explicit SmlMockUi(const std::string& sml_source) {
        const auto doc = smlcore::parse_document(sml_source);
        collect_ids(doc.roots);
        g_mock           = &state;
        g_sml_known_ids  = &known_ids;
        g_sml_init_props = &init_props;
        sms_native_set_ui_callbacks(sml_ui_get, mock_ui_set, mock_ui_invoke,
                                    nullptr, 0);
    }

    ~SmlMockUi() {
        sms_native_set_ui_callbacks(nullptr, nullptr, nullptr, nullptr, 0);
        g_sml_init_props = nullptr;
        g_sml_known_ids  = nullptr;
        g_mock           = nullptr;
    }

private:
    void collect_ids(const std::vector<smlcore::Node>& nodes) {
        for (const auto& node : nodes) {
            const auto* id_prop = node.find_property("id");
            if (id_prop) {
                known_ids.insert(id_prop->value);
                // Expose other SML attributes as initial property values.
                for (const auto& prop : node.properties) {
                    if (prop.name == "id") continue;
                    std::string json_val;
                    switch (prop.kind) {
                        case smlcore::ValueKind::Number:
                        case smlcore::ValueKind::Bool:
                            json_val = prop.value;
                            break;
                        default:
                            json_val = "\"" + prop.value + "\"";
                            break;
                    }
                    init_props[id_prop->value + "." + prop.name] = json_val;
                }
            }
            collect_ids(node.children);
        }
    }
};

// ==========================================================================
// Dispatch Depth Guard - mirrors SmsBridge::dispatch_event without Godot
// ==========================================================================

static constexpr int kTestMaxDispatchDepth = 16;

struct DispatchHarness {
    SmsSession& session;
    int         depth           = 0;
    bool        guard_triggered = false;
    std::string last_error;

    explicit DispatchHarness(SmsSession& s) : session(s) {}

    std::string dispatch(const std::string& target, const std::string& event,
                         const std::string& args = "[]") {
        if (depth >= kTestMaxDispatchDepth) {
            guard_triggered = true;
            last_error = "RuntimeError: SMS dispatch recursion limit exceeded for '"
                + target + "." + event + "' (possible stack overflow).";
            return last_error;
        }
        ++depth;
        const std::string err = session.invoke(target, event, args);
        --depth;
        last_error = err;
        return err;
    }
};

// ==========================================================================
// Re-entrant dispatch state for the setKeyframe cycle test
// ==========================================================================

static std::int64_t g_cycle_session_id  = -1;
static int          g_cycle_depth       = 0;
static bool         g_cycle_guard_hit   = false;
static int          g_setKeyframe_calls = 0;
static std::string  g_cycle_engine_error;

// --------------------------------------------------------------------------
// Unguarded re-entrancy state - used by test_setKeyframe_cycle_unprotected
//
// Unlike mock_invoke_cycling this mock has NO depth guard of its own.
// It only has a hard safety stopper (kUnguardedSafetyLimit) to prevent an
// actual OS crash in case the engine does NOT detect re-entrancy.
//
// Expected behaviour after Codex fixes the engine:
//   sms_native_session_invoke returns a non-empty error string when called
//   re-entrantly → the test goes GREEN.
//
// Current behaviour (engine has the bug):
//   sms_native_session_invoke recurses silently → error string stays empty
//   → the test assertion fails → RED.
// --------------------------------------------------------------------------

static constexpr int kUnguardedSafetyLimit = 30;   // absolute crash-prevention stopper
static int           g_unguarded_calls     = 0;
static std::string   g_unguarded_engine_error;      // first non-empty error from engine

static int mock_invoke_unguarded(const char* id, const char* method, const char* args,
                                  char* out, int cap) {
    if (out && cap > 0) out[0] = '\0';
    const std::string obj  = id     ? id     : "";
    const std::string meth = method ? method : "";

    if (obj == "timeline" && meth == "setKeyframe") {
        ++g_unguarded_calls;
        if (g_unguarded_calls >= kUnguardedSafetyLimit) return 0; // crash prevention only
        std::int64_t result = 0;
        char err[512] = {};
        sms_native_session_invoke(g_cycle_session_id,
                                  "timeline", "keyframeAdded",
                                  "[7,\"Spine\"]",
                                  &result, err, static_cast<int>(sizeof(err)));
        if (err[0] && g_unguarded_engine_error.empty())
            g_unguarded_engine_error = err;

    } else if (obj == "editor" && meth == "poseChanged") {
        ++g_unguarded_calls;
        if (g_unguarded_calls >= kUnguardedSafetyLimit) return 0; // crash prevention only
        std::int64_t result = 0;
        char err[512] = {};
        sms_native_session_invoke(g_cycle_session_id,
                                  "editor", "poseChanged",
                                  args ? args : "[\"Spine\"]",
                                  &result, err, static_cast<int>(sizeof(err)));
        if (err[0] && g_unguarded_engine_error.empty())
            g_unguarded_engine_error = err;
    }
    return 0;
}

// Simulates the ForgePoser crash scenario:
//   timeline.setKeyframe  → C++ emits keyframeAdded → re-dispatch into SMS
//   editor.poseChanged()  → C++ emits poseChanged   → re-dispatch into SMS
// The mock has its own depth guard, but with the engine re-entrancy guard in place
// the nested dispatch should fail earlier and be captured in g_cycle_engine_error.
static int mock_invoke_cycling(const char* id, const char* method, const char* args,
                                char* out, int cap) {
    if (out && cap > 0) out[0] = '\0';
    const std::string obj  = id     ? id     : "";
    const std::string meth = method ? method : "";

    if (obj == "timeline" && meth == "setKeyframe") {
        ++g_setKeyframe_calls;
        if (g_cycle_depth >= kTestMaxDispatchDepth) {
            g_cycle_guard_hit = true;
            return 0;
        }
        ++g_cycle_depth;
        std::int64_t result = 0;
        char err[512] = {};
        // Simulate C++ set_keyframe emitting keyframeAdded signal → SMS dispatch
        sms_native_session_invoke(g_cycle_session_id,
                                  "timeline", "keyframeAdded",
                                  "[7,\"Spine\"]",
                                  &result, err, static_cast<int>(sizeof(err)));
        if (err[0] && g_cycle_engine_error.empty())
            g_cycle_engine_error = err;
        --g_cycle_depth;

    } else if (obj == "editor" && meth == "poseChanged") {
        if (g_cycle_depth >= kTestMaxDispatchDepth) {
            g_cycle_guard_hit = true;
            return 0;
        }
        ++g_cycle_depth;
        std::int64_t result = 0;
        char err[512] = {};
        // Simulate C++ poseChanged method emitting poseChanged signal → SMS dispatch
        sms_native_session_invoke(g_cycle_session_id,
                                  "editor", "poseChanged",
                                  args ? args : "[\"Spine\"]",
                                  &result, err, static_cast<int>(sizeof(err)));
        if (err[0] && g_cycle_engine_error.empty())
            g_cycle_engine_error = err;
        --g_cycle_depth;
    }
    return 0;
}

// ==========================================================================
// Tests
// ==========================================================================

// Baseline: SMS loads, event fires, property is set via mock callback.
// SMS objects are string handles (e.g. var label = "label") - the engine
// routes method/property calls on strings through the UI callbacks.
void test_basic_dispatch() {
    ScopedMockUi mock;

    SmsSession session;
    std::string err;
    assert_true(session.load(R"(
        on button.clicked() {
            var label = ui.getObject("label")
            label.text = "hello"
        }
    )", err), "load failed: " + err);

    const std::string dispatch_err = session.invoke("button", "clicked");
    assert_true(dispatch_err.empty(), "unexpected error: " + dispatch_err);

    const auto it = mock.state.props.find("label.text");
    assert_true(it != mock.state.props.end(), "label.text was not set by SMS");
    assert_contains(it->second, "hello", "label.text value");
}

// Missing handler returns a classifiable error; does NOT require exit.
void test_no_handler_is_silent() {
    ScopedMockUi mock;

    SmsSession session;
    std::string err;
    assert_true(session.load("on button.clicked() { }", err), "load: " + err);

    const std::string dispatch_err = session.invoke("nonexistent", "event");
    assert_true(forge::sms_error_is_missing_handler(dispatch_err),
                "expected missing-handler, got: \"" + dispatch_err + "\"");
    assert_false(forge::sms_error_requires_exit(dispatch_err),
                 "missing handler must not require exit");
}

// Pure SMS recursion (fun calling itself).
// The SMS engine must return a proper error string rather than crashing.
void test_direct_script_recursion_caught() {
    ScopedMockUi mock;

    SmsSession session;
    std::string err;
    assert_true(session.load(R"(
        fun recurse() {
            return recurse()
        }
        on trigger.fire() {
            recurse()
        }
    )", err), "load: " + err);

    const std::string dispatch_err = session.invoke("trigger", "fire");
    assert_false(dispatch_err.empty(),
                 "expected recursion error, got empty string (engine may have crashed)");
    assert_true(forge::sms_error_requires_exit(dispatch_err),
                "SMS recursion error must require exit, got: \"" + dispatch_err + "\"");
}

// DispatchHarness depth guard fires when depth is pre-saturated.
void test_dispatch_depth_guard_fires() {
    ScopedMockUi mock;

    SmsSession session;
    std::string err;
    assert_true(session.load(R"(
        on ticker.tick() {
            var label = ui.getObject("label")
            label.text = "tick"
        }
    )", err), "load: " + err);

    DispatchHarness harness(session);
    harness.depth = kTestMaxDispatchDepth;  // pre-saturate

    const std::string guard_err = harness.dispatch("ticker", "tick");
    assert_true(harness.guard_triggered, "depth guard should have triggered");
    assert_true(forge::sms_error_requires_exit(guard_err),
                "depth guard error must require exit, got: \"" + guard_err + "\"");
}

// Reproduces the ForgePoser crash (Timeline.setKeyframe cid='char2' frame=7).
//
// Cycle:
//   editor.poseChanged (SMS event)
//     → timeline.setKeyframe(1, null)  [mock re-dispatches keyframeAdded]
//     → timeline.keyframeAdded (SMS event)
//       → editor.poseChanged(bone)     [mock re-dispatches poseChanged]
//       → editor.poseChanged (SMS event) → ...
//
// The engine-level re-entrancy guard must stop the cycle before the mock
// depth guard is needed.
void test_setKeyframe_cycle_caught() {
    g_cycle_depth       = 0;
    g_cycle_guard_hit   = false;
    g_setKeyframe_calls = 0;
    g_cycle_engine_error.clear();

    ScopedMockUi mock;
    mock.state.invoke_hook = mock_invoke_cycling;

    SmsSession session;
    std::string err;
    assert_true(session.load(R"(
        on editor.poseChanged(bone) {
            var timeline = ui.getObject("timeline")
            timeline.setKeyframe(1, null)
        }
        on timeline.keyframeAdded(frame, bone) {
            var editorRef = ui.getObject("editor")
            editorRef.poseChanged(bone)
        }
    )", err), "load: " + err);

    g_cycle_session_id = session.id;
    struct Cleanup {
        ~Cleanup() { g_cycle_session_id = -1; }
    } cleanup;

    session.invoke("editor", "poseChanged", "[\"Spine\"]");

    assert_false(g_cycle_engine_error.empty(),
                 "engine must detect re-entrant sms_native_session_invoke in setKeyframe cycle");
    assert_true(forge::sms_error_requires_exit(g_cycle_engine_error),
                "re-entrancy error must require exit, got: \"" + g_cycle_engine_error + "\"");
    assert_false(g_cycle_guard_hit,
                 "mock depth guard should not be needed once engine guard triggers");
    assert_true(g_setKeyframe_calls > 0,
                "setKeyframe must have been called at least once");
    assert_true(g_setKeyframe_calls <= 2,
                "cycle must stop immediately after re-entrant invoke is rejected (calls: "
                + std::to_string(g_setKeyframe_calls) + ")");
}

// ---------------------------------------------------------------------------
// RED TEST - turns GREEN after the engine fix described in:
//   CWUP/tasks/sms_native_reentrant_invoke_guard.md
//
// What to implement (summary for Codex):
//
//   Add a per-session re-entrancy flag to the Session struct in
//   SMSCore.Native/src/sms_native.cpp and check it at the top of
//   sms_native_session_invoke():
//
//     struct Session {
//         bool invoking = false;   // ← add this
//         // ... existing fields ...
//     };
//
//     int sms_native_session_invoke(int64_t session_id, ...) {
//         Session* s = find_session(session_id);
//         if (s->invoking) {
//             snprintf(out_error, out_error_cap,
//                 "RuntimeError: re-entrant sms_native_session_invoke for "
//                 "session %" PRId64 " - a UI callback triggered a new "
//                 "dispatch while a handler was executing.", session_id);
//             return 1;
//         }
//         struct InvokeGuard { bool& f; ~InvokeGuard(){f=false;} } g{s->invoking};
//         s->invoking = true;
//         // ... rest of existing invoke logic unchanged ...
//     }
//
//   The error string MUST contain "RuntimeError:" so that
//   forge::sms_error_requires_exit() classifies it correctly and
//   ForgeRunner.Native terminates cleanly.
//
//   No other files need to change.
//   See the task file for Option B (thread_local) and full rationale.
// ---------------------------------------------------------------------------
//
// Why the existing kMaxSmsDispatchDepth guard is not enough:
//   SmsBridge::dispatch_event in forge_runner_extension.cpp has a
//   thread_local depth counter, but it lives in the Godot extension layer -
//   direct callers of sms_native_session_invoke (like this test harness)
//   bypass it entirely. 256 nested C frames can also overflow the OS stack
//   before the counter fires. The engine-level guard rejects on call #2.
// ---------------------------------------------------------------------------
void test_setKeyframe_cycle_unprotected() {
    g_unguarded_calls       = 0;
    g_unguarded_engine_error.clear();

    ScopedMockUi mock;
    mock.state.invoke_hook = mock_invoke_unguarded;

    SmsSession session;
    std::string err;
    assert_true(session.load(R"(
        on editor.poseChanged(bone) {
            var timeline = ui.getObject("timeline")
            timeline.setKeyframe(1, null)
        }
        on timeline.keyframeAdded(frame, bone) {
            var editorRef = ui.getObject("editor")
            editorRef.poseChanged(bone)
        }
    )", err), "load: " + err);

    g_cycle_session_id = session.id;
    struct Cleanup { ~Cleanup() { g_cycle_session_id = -1; } } cleanup;

    // Fire the first dispatch - if the engine has no re-entrancy guard this
    // recurses until the safety stopper fires; no error is ever returned.
    session.invoke("editor", "poseChanged", "[\"Spine\"]");

    assert_false(g_unguarded_engine_error.empty(),
        "engine must detect re-entrant sms_native_session_invoke and return an error "
        "(cycled " + std::to_string(g_unguarded_calls) + " times without protection)");
    assert_true(forge::sms_error_requires_exit(g_unguarded_engine_error),
        "re-entrancy error must require exit, got: \"" + g_unguarded_engine_error + "\"");
}

// 100 sequential dispatches of the same event must never trigger the guard.
void test_sequential_dispatches_ok() {
    ScopedMockUi mock;

    SmsSession session;
    std::string err;
    assert_true(session.load(R"(
        on btn.clicked() {
            var counter = ui.getObject("counter")
            counter.text = "clicked"
        }
    )", err), "load: " + err);

    DispatchHarness harness(session);
    for (int i = 0; i < 100; ++i) {
        harness.dispatch("btn", "clicked");
        assert_false(harness.guard_triggered,
                     "depth guard triggered unexpectedly on sequential dispatch "
                     + std::to_string(i));
    }
}

// Error strings produced by the depth guard / SMS engine must require exit.
void test_error_policy_on_overflow_message() {
    const std::string overflow_msg =
        "RuntimeError: SMS dispatch recursion limit exceeded for "
        "'timeline.keyframeAdded' (possible stack overflow).";
    assert_true(forge::sms_error_requires_exit(overflow_msg),
                "dispatch overflow must require exit");
    assert_false(forge::sms_error_is_missing_handler(overflow_msg),
                 "dispatch overflow is not a missing-handler error");

    // Also the plain OS-level message that triggered the original crash
    const std::string os_overflow = "Stack overflow.";
    assert_true(forge::sms_error_requires_exit(os_overflow),
                "plain Stack overflow. must require exit");
}

// Property round-trip: SMS reads a value and writes it to another object.
void test_property_get_set_round_trip() {
    ScopedMockUi mock;
    mock.state.props["sourceLabel.text"] = "\"42\"";

    SmsSession session;
    std::string err;
    assert_true(session.load(R"(
        on form.submit() {
            var sourceLabel = ui.getObject("sourceLabel")
            var resultLabel = ui.getObject("resultLabel")
            var v = sourceLabel.text
            resultLabel.text = v
        }
    )", err), "load: " + err);

    const std::string dispatch_err = session.invoke("form", "submit");
    assert_true(dispatch_err.empty(), "unexpected error: " + dispatch_err);

    const auto it = mock.state.props.find("resultLabel.text");
    assert_true(it != mock.state.props.end(), "resultLabel.text was not written");
}

// ==========================================================================
// SML + SMS integration tests
//
// These tests document the full pipeline from SML element declaration to
// SMS property access, which is how ForgeRunner.Native processes apps:
//
//   SML:  <label id="statusLabel" text="Ready" />
//           → element registered, id known, attribute "text" exposed
//   SMS:  var lbl = ui.getObject("statusLabel")
//           → __ui_ref{id="statusLabel"} created because __exists == "1"
//         lbl.text = "Saved"
//           → sms_ui_set("statusLabel", "text", "\"Saved\"", ...)
// ==========================================================================

// SML declares a label; SMS sets its text via ui.getObject().
void test_sml_known_id_accessible() {
    SmlMockUi mock(R"(
        Screen {
            Label {
                id: statusLabel
                text: "Ready"
            }
        }
    )");

    SmsSession session;
    std::string err;
    assert_true(session.load(R"(
        on btnSave.clicked() {
            var lbl = ui.getObject("statusLabel")
            lbl.text = "Saved"
        }
    )", err), "load: " + err);

    const std::string dispatch_err = session.invoke("btnSave", "clicked");
    assert_true(dispatch_err.empty(), "unexpected error: " + dispatch_err);

    const auto it = mock.state.props.find("statusLabel.text");
    assert_true(it != mock.state.props.end(),
                "statusLabel.text was not written by SMS");
    assert_contains(it->second, "Saved", "statusLabel.text");
}

// Requesting an ID not in the SML returns null; null guard in SMS works.
void test_sml_unknown_id_is_null() {
    SmlMockUi mock(R"(
        Screen {
            Label {
                id: statusLabel
            }
        }
    )");

    SmsSession session;
    std::string err;
    assert_true(session.load(R"(
        on btn.clicked() {
            var ghost = ui.getObject("nonExistentId")
            if (ghost != null) {
                ghost.text = "should not happen"
            }
            var lbl = ui.getObject("statusLabel")
            lbl.text = "ok"
        }
    )", err), "load: " + err);

    const std::string dispatch_err = session.invoke("btn", "clicked");
    assert_true(dispatch_err.empty(), "unexpected error: " + dispatch_err);

    // ghost was null → its text must NOT have been set
    assert_true(mock.state.props.find("nonExistentId.text") == mock.state.props.end(),
                "null-guard failed: ghost.text was written");
    // statusLabel was found → its text must have been set
    const auto it = mock.state.props.find("statusLabel.text");
    assert_true(it != mock.state.props.end(), "statusLabel.text was not written");
}

// SML attribute values (e.g. value="42") are readable as initial properties.
void test_sml_initial_attribute_readable() {
    SmlMockUi mock(R"(
        Screen {
            Slider {
                id: vol
                value: 42
            }
            Label {
                id: display
            }
        }
    )");

    SmsSession session;
    std::string err;
    assert_true(session.load(R"(
        on form.load() {
            var vol     = ui.getObject("vol")
            var display = ui.getObject("display")
            display.text = vol.value
        }
    )", err), "load: " + err);

    const std::string dispatch_err = session.invoke("form", "load");
    assert_true(dispatch_err.empty(), "unexpected error: " + dispatch_err);

    const auto it = mock.state.props.find("display.text");
    assert_true(it != mock.state.props.end(), "display.text was not written");
    assert_contains(it->second, "42", "display.text should reflect slider value");
}

// Full SML layout + SMS event handler: button click updates a label.
// This is the canonical "hello world" of the SML→SMS pipeline.
void test_sml_button_click_updates_label() {
    SmlMockUi mock(R"(
        Screen {
            VBoxContainer {
                Button {
                    id: btnSave
                    text: "Save"
                }
                Label {
                    id: feedback
                    text: ""
                }
            }
        }
    )");

    SmsSession session;
    std::string err;
    assert_true(session.load(R"(
        on btnSave.clicked() {
            var feedback = ui.getObject("feedback")
            feedback.text = "Project saved!"
        }
    )", err), "load: " + err);

    const std::string dispatch_err = session.invoke("btnSave", "clicked");
    assert_true(dispatch_err.empty(), "unexpected error: " + dispatch_err);

    const auto it = mock.state.props.find("feedback.text");
    assert_true(it != mock.state.props.end(), "feedback.text was not written");
    assert_contains(it->second, "Project saved!", "feedback.text");
}

// ==========================================================================
// Test registry + main
// ==========================================================================

using TestFn = void(*)();
struct TestCase { const char* name; TestFn fn; };

const std::vector<TestCase>& all_tests() {
    static const std::vector<TestCase> tests = {
        { "runner_integration_basic_dispatch",
          test_basic_dispatch },
        { "runner_integration_no_handler_is_silent",
          test_no_handler_is_silent },
        { "runner_integration_direct_script_recursion_caught",
          test_direct_script_recursion_caught },
        { "runner_integration_dispatch_depth_guard_fires",
          test_dispatch_depth_guard_fires },
        { "runner_integration_setKeyframe_cycle_caught",
          test_setKeyframe_cycle_caught },
        { "runner_integration_setKeyframe_cycle_unprotected",   // RED until engine fix
          test_setKeyframe_cycle_unprotected },
        { "runner_integration_sequential_dispatches_ok",
          test_sequential_dispatches_ok },
        { "runner_integration_error_policy_on_overflow",
          test_error_policy_on_overflow_message },
        { "runner_integration_property_get_set_round_trip",
          test_property_get_set_round_trip },
        // SML + SMS pipeline tests
        { "runner_sml_known_id_accessible",
          test_sml_known_id_accessible },
        { "runner_sml_unknown_id_is_null",
          test_sml_unknown_id_is_null },
        { "runner_sml_initial_attribute_readable",
          test_sml_initial_attribute_readable },
        { "runner_sml_button_click_updates_label",
          test_sml_button_click_updates_label },
    };
    return tests;
}

} // namespace

int main(int argc, char** argv) {
    try {
        const auto& tests = all_tests();

        if (argc == 2) {
            const std::string requested = argv[1];
            for (const auto& t : tests) {
                if (requested == t.name) {
                    t.fn();
                    std::cout << "forge_runner_native_sms_integration_tests: "
                              << t.name << " passed\n";
                    return 0;
                }
            }
            throw std::runtime_error("unknown test: " + requested);
        }

        for (const auto& t : tests) {
            t.fn();
        }
        std::cout << "forge_runner_native_sms_integration_tests: all tests passed ("
                  << tests.size() << ")\n";
        return 0;

    } catch (const std::exception& ex) {
        std::cerr << "forge_runner_native_sms_integration_tests failed: "
                  << ex.what() << "\n";
        return 1;
    }
}
