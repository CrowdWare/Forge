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

#include "sms_native.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace {

static const std::string kMantra = "Tu keinem Lebewesen Leid an.\n";

void assert_true(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

struct MockState {
    std::unordered_map<std::string, std::string> json_props;
    std::unordered_map<std::string, std::string> text_props;
    int json_get_calls = 0;
    int json_set_calls = 0;
    int raw_get_calls = 0;
    int raw_set_calls = 0;
};

MockState* g_state = nullptr;

static int mock_ui_get(const char* object_id, const char* property, char* out_json, int out_json_capacity, char*, int) {
    if (g_state == nullptr) return 1;
    g_state->json_get_calls++;
    const std::string id = object_id ? object_id : "";
    const std::string prop = property ? property : "";
    if (prop == "__exists") {
        std::snprintf(out_json, static_cast<std::size_t>(out_json_capacity), "1");
        return 0;
    }
    const std::string key = id + "." + prop;
    auto it = g_state->json_props.find(key);
    if (it == g_state->json_props.end()) {
        std::snprintf(out_json, static_cast<std::size_t>(out_json_capacity), "null");
    } else {
        std::snprintf(out_json, static_cast<std::size_t>(out_json_capacity), "%s", it->second.c_str());
    }
    return 0;
}

static int mock_ui_set(const char* object_id, const char* property, const char* value_json, char*, int) {
    if (g_state == nullptr) return 1;
    g_state->json_set_calls++;
    const std::string key = std::string(object_id ? object_id : "") + "." + (property ? property : "");
    g_state->json_props[key] = value_json ? value_json : "null";
    return 0;
}

static int mock_ui_invoke(const char*, const char*, const char*, char* out_json, int out_json_capacity, char*, int) {
    if (out_json != nullptr && out_json_capacity > 0) {
        std::snprintf(out_json, static_cast<std::size_t>(out_json_capacity), "null");
    }
    return 0;
}

static int mock_ui_get_string(const char* object_id, const char* property, char* out_text, int out_text_capacity, char* error, int error_capacity) {
    if (g_state == nullptr) return 1;
    g_state->raw_get_calls++;
    const std::string key = std::string(object_id ? object_id : "") + "." + (property ? property : "");
    auto it = g_state->text_props.find(key);
    if (it == g_state->text_props.end()) {
        if (error != nullptr && error_capacity > 0) {
            std::snprintf(error, static_cast<std::size_t>(error_capacity), "raw text key not found: %s", key.c_str());
        }
        return 1;
    }
    std::snprintf(out_text, static_cast<std::size_t>(out_text_capacity), "%s", it->second.c_str());
    return 0;
}

static int mock_ui_set_string(const char* object_id, const char* property, const char* value_text, char*, int) {
    if (g_state == nullptr) return 1;
    g_state->raw_set_calls++;
    const std::string key = std::string(object_id ? object_id : "") + "." + (property ? property : "");
    g_state->text_props[key] = value_text ? value_text : "";
    return 0;
}

struct ScopedCallbacks {
    explicit ScopedCallbacks(MockState& state, bool enable_raw_callbacks) {
        g_state = &state;
        sms_native_set_ui_callbacks(&mock_ui_get, &mock_ui_set, &mock_ui_invoke, nullptr, 0);
        if (enable_raw_callbacks) {
            sms_native_set_ui_string_callbacks(&mock_ui_get_string, &mock_ui_set_string, nullptr, 0);
        } else {
            sms_native_set_ui_string_callbacks(nullptr, nullptr, nullptr, 0);
        }
    }

    ~ScopedCallbacks() {
        sms_native_set_ui_string_callbacks(nullptr, nullptr, nullptr, 0);
        sms_native_set_ui_callbacks(nullptr, nullptr, nullptr, nullptr, 0);
        g_state = nullptr;
    }
};

struct SmsSession {
    std::int64_t id = -1;

    bool load(const std::string& source, std::string& out_error) {
        const std::string full = kMantra + source;
        char error[1024] = {};
        if (sms_native_session_create(&id, error, static_cast<int>(sizeof(error))) != 0 || id < 0) {
            out_error = error;
            return false;
        }
        if (sms_native_session_load(id, full.c_str(), error, static_cast<int>(sizeof(error))) != 0) {
            out_error = error;
            dispose();
            return false;
        }
        return true;
    }

    std::string invoke(const std::string& target, const std::string& event, const std::string& args = "[]") {
        std::int64_t result = 0;
        char error[1024] = {};
        sms_native_session_invoke(id, target.c_str(), event.c_str(), args.c_str(),
                                  &result, error, static_cast<int>(sizeof(error)));
        return std::string(error);
    }

    void dispose() {
        if (id >= 0) {
            sms_native_session_dispose(id, nullptr, 0);
            id = -1;
        }
    }

    ~SmsSession() { dispose(); }
};

void test_ui_text_roundtrip_uses_raw_callbacks() {
    MockState state;
    state.text_props["label.text"] = "initial";
    ScopedCallbacks callbacks(state, true);

    SmsSession session;
    std::string load_error;
    assert_true(session.load(R"(
        on button.clicked() {
            var label = ui.getObject("label")
            var result = ui.getObject("result")
            label.text = "Hello
Über"
            var t = label.text
            result.text = t
        }
    )", load_error), "load failed: " + load_error);

    const std::string invoke_error = session.invoke("button", "clicked");
    assert_true(invoke_error.empty(), "invoke failed: " + invoke_error);

    assert_true(state.raw_set_calls >= 2, "raw set callback should be used for text assignments");
    assert_true(state.raw_get_calls >= 1, "raw get callback should be used for text reads");
    assert_true(state.json_set_calls == 0, "json set callback should not be used for text when raw callbacks are present");
    assert_true(state.text_props["result.text"] == "Hello\nÜber", "result.text should keep multiline and umlaut text");
}

void test_ui_text_roundtrip_falls_back_to_json_without_raw_callbacks() {
    MockState state;
    state.json_props["label.text"] = "\"initial\"";
    ScopedCallbacks callbacks(state, false);

    SmsSession session;
    std::string load_error;
    assert_true(session.load(R"(
        on button.clicked() {
            var label = ui.getObject("label")
            var result = ui.getObject("result")
            label.text = "Hello
Über"
            var t = label.text
            result.text = t
        }
    )", load_error), "load failed: " + load_error);

    const std::string invoke_error = session.invoke("button", "clicked");
    assert_true(invoke_error.empty(), "invoke failed: " + invoke_error);

    assert_true(state.raw_set_calls == 0, "raw set callback should not be used when not registered");
    assert_true(state.raw_get_calls == 0, "raw get callback should not be used when not registered");
    assert_true(state.json_set_calls >= 2, "json set callback should be used as fallback");
    auto it = state.json_props.find("result.text");
    assert_true(it != state.json_props.end(), "result.text should be set via JSON fallback");
    assert_true(it->second.find("Hello\\nÜber") != std::string::npos, "fallback JSON payload should keep escaped newline");
}

using TestFn = void(*)();
struct TestCase { const char* name; TestFn fn; };

const std::vector<TestCase>& all_tests() {
    static const std::vector<TestCase> tests = {
        {"ui_string_bridge_text_roundtrip_uses_raw_callbacks", test_ui_text_roundtrip_uses_raw_callbacks},
        {"ui_string_bridge_text_roundtrip_falls_back_to_json_without_raw_callbacks", test_ui_text_roundtrip_falls_back_to_json_without_raw_callbacks},
    };
    return tests;
}

} // namespace

int main(int argc, char** argv) {
    try {
        const auto& tests = all_tests();
        if (argc == 2) {
            const std::string requested = argv[1];
            for (const auto& test : tests) {
                if (requested == test.name) {
                    test.fn();
                    std::cout << "sms_native_ui_string_bridge_tests: " << test.name << " passed\n";
                    return 0;
                }
            }
            throw std::runtime_error("unknown test case: " + requested);
        }

        for (const auto& test : tests) {
            test.fn();
        }
        std::cout << "sms_native_ui_string_bridge_tests: all tests passed (" << tests.size() << ")\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "sms_native_ui_string_bridge_tests failed: " << ex.what() << "\n";
        return 1;
    }
}

