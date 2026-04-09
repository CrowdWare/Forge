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

#include "forge_sms_bridge.h"
#include "forge_sms_error_policy.h"
#include "forge_json_string.h"
#include "forge_path_resolver.h"
#include "sms_native.h"

#include <algorithm>
#include <atomic>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <chrono>
#include <deque>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits>
#include <memory>
#include <mutex>
#include <sstream>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#if defined(_WIN32)
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/base_button.hpp>
#include <godot_cpp/classes/check_box.hpp>
#include <godot_cpp/classes/check_button.hpp>
#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/item_list.hpp>
#include <godot_cpp/classes/json.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/line_edit.hpp>
#include <godot_cpp/classes/main_loop.hpp>
#include <godot_cpp/classes/option_button.hpp>
#include <godot_cpp/classes/progress_bar.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/rich_text_label.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/scroll_container.hpp>
#include <godot_cpp/classes/slider.hpp>
#include <godot_cpp/classes/spin_box.hpp>
#include <godot_cpp/classes/text_edit.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/tree.hpp>
#include <godot_cpp/classes/tree_item.hpp>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace fs = std::filesystem;
using namespace godot;

namespace forge {

namespace {
constexpr int kMaxSmsDispatchDepth = 256;
enum class SmsRuntimeMode : int {
    Interpreter = 0,
    Native = 1
};
std::atomic<int> g_sms_runtime_mode{static_cast<int>(SmsRuntimeMode::Interpreter)};

void set_sms_runtime_mode(SmsRuntimeMode mode) {
    g_sms_runtime_mode.store(static_cast<int>(mode), std::memory_order_relaxed);
}

const char* current_sms_runtime_mode_name() {
    const int raw = g_sms_runtime_mode.load(std::memory_order_relaxed);
    return raw == static_cast<int>(SmsRuntimeMode::Native) ? "native" : "interpreter";
}

bool starts_with_http_scheme(const std::string& label) {
    return label.rfind("http://", 0) == 0 || label.rfind("https://", 0) == 0;
}

bool is_dev_mode() {
    const char* env = std::getenv("FORGE_DEV_MODE");
    return env != nullptr && env[0] != '\0' && env[0] != '0';
}

bool runner_started_from_http_url() {
    const char* env = std::getenv("FORGE_RUNNER_URL");
    if (env == nullptr || env[0] == '\0') {
        return false;
    }
    return starts_with_http_scheme(std::string(env));
}

bool is_local_source_label(const std::string& source_label) {
    if (runner_started_from_http_url()) {
        // Remote apps are downloaded to local cache paths, but must stay on interpreter path.
        return false;
    }
    if (source_label.empty()) {
        return true;
    }
    if (source_label.rfind("file://", 0) == 0) {
        return true;
    }
    if (starts_with_http_scheme(source_label)) {
        return false;
    }
    return true;
}

bool has_sms_event_handlers(const std::string& source) {
    // Heuristic: detect "on <object>.<event>(...)" declarations outside comments/strings.
    bool in_line_comment = false;
    bool in_string = false;
    bool saw_on_keyword = false;
    for (std::size_t i = 0; i < source.size(); ++i) {
        const char ch = source[i];
        const char next = (i + 1 < source.size()) ? source[i + 1] : '\0';

        if (in_line_comment) {
            if (ch == '\n') {
                in_line_comment = false;
            }
            continue;
        }
        if (in_string) {
            if (ch == '"' && (i == 0 || source[i - 1] != '\\')) {
                in_string = false;
            }
            continue;
        }
        if (ch == '/' && next == '/') {
            in_line_comment = true;
            ++i;
            continue;
        }
        if (ch == '"') {
            in_string = true;
            continue;
        }
        if (ch != 'o' || next != 'n') {
            continue;
        }

        const char before = (i > 0) ? source[i - 1] : ' ';
        if (std::isalnum(static_cast<unsigned char>(before)) || before == '_') {
            continue;
        }
        saw_on_keyword = true;

        std::size_t j = i + 2;
        while (j < source.size() && std::isspace(static_cast<unsigned char>(source[j]))) {
            ++j;
        }
        if (j >= source.size() || !(std::isalpha(static_cast<unsigned char>(source[j])) || source[j] == '_')) {
            continue;
        }
        // object id
        while (j < source.size() && (std::isalnum(static_cast<unsigned char>(source[j])) || source[j] == '_')) {
            ++j;
        }
        while (j < source.size() && std::isspace(static_cast<unsigned char>(source[j]))) {
            ++j;
        }
        if (j >= source.size() || source[j] != '.') {
            continue;
        }
        ++j; // '.'
        while (j < source.size() && std::isspace(static_cast<unsigned char>(source[j]))) {
            ++j;
        }
        if (j >= source.size() || !(std::isalpha(static_cast<unsigned char>(source[j])) || source[j] == '_')) {
            continue;
        }
        // event name
        while (j < source.size() && (std::isalnum(static_cast<unsigned char>(source[j])) || source[j] == '_')) {
            ++j;
        }
        while (j < source.size() && std::isspace(static_cast<unsigned char>(source[j]))) {
            ++j;
        }
        if (j < source.size() && source[j] == '(') {
            return true; // on object.event(...)
        }
    }
    // Conservative fallback: if "on" keyword is present, keep interpreter path.
    return saw_on_keyword;
}

bool try_quit_on_fatal_sms_error(const std::string& message) {
    if (!sms_error_requires_exit(message)) {
        return false;
    }

    UtilityFunctions::printerr(String(("[ForgeRunner.Native] Fatal RuntimeError. Exiting: " + message).c_str()));
    auto* main_loop = Engine::get_singleton() ? Engine::get_singleton()->get_main_loop() : nullptr;
    auto* tree = Object::cast_to<SceneTree>(main_loop);
    if (tree != nullptr) {
        tree->quit(1);
    }
    return true;
}
} // namespace

// ---------------------------------------------------------------------------
// Global id map
// ---------------------------------------------------------------------------

IdMap& SmsBridge::id_map() {
    static IdMap s_map;
    return s_map;
}

static UiOpenDialogHook& ui_open_dialog_hook() {
    static UiOpenDialogHook hook = nullptr;
    return hook;
}

static std::string json_string(const std::string& s);

void set_ui_open_dialog_hook(UiOpenDialogHook hook) {
    ui_open_dialog_hook() = hook;
}

namespace {
struct BenchmarkJob {
    std::string name;
    std::atomic<bool> cancel{false};
    std::atomic<int> worker_a_progress{0};
    std::atomic<int> worker_b_progress{0};
    std::atomic<bool> done{false};
    std::int64_t duration_ms = 0;
    int sms_score = 0;
    int kotlin_score = 1000;
    std::thread worker_a;
    std::thread worker_b;
    std::thread monitor;
};

std::mutex g_benchmark_mutex;
std::unique_ptr<BenchmarkJob> g_benchmark_job;
std::mutex g_benchmark_events_mutex;
std::deque<std::pair<std::string, std::string>> g_benchmark_events;

int benchmark_workload_units(const std::string& name) {
    if (name == "cpu") return 18;
    if (name == "dispatch") return 12;
    if (name == "frame") return 14;
    if (name == "mixed") return 20;
    return 10;
}

int benchmark_kotlin_score(const std::string& name) {
    if (name == "cpu") return 1000;
    if (name == "dispatch") return 900;
    if (name == "frame") return 950;
    if (name == "mixed") return 980;
    return 1000;
}

std::string benchmark_progress_payload(const std::string& name, int percent, const std::string& stage) {
    return "[" + json_string(name) + "," + std::to_string(percent) + "," + json_string(stage) + "]";
}

std::string benchmark_completed_payload(
    const std::string& name, std::int64_t duration_ms, int sms_score, int kotlin_score, const std::string& summary) {
    return "[" + json_string(name) + "," + std::to_string(duration_ms) + ","
        + std::to_string(sms_score) + "," + std::to_string(kotlin_score) + "," + json_string(summary) + "]";
}

void enqueue_benchmark_event(const std::string& event_name, const std::string& payload_json) {
    std::lock_guard<std::mutex> lock(g_benchmark_events_mutex);
    g_benchmark_events.emplace_back(event_name, payload_json);
}

void join_job_threads(BenchmarkJob& job) {
    if (job.worker_a.joinable()) job.worker_a.join();
    if (job.worker_b.joinable()) job.worker_b.join();
    if (job.monitor.joinable()) job.monitor.join();
}

void stop_active_benchmark_job_locked() {
    if (!g_benchmark_job) return;
    g_benchmark_job->cancel.store(true);
    join_job_threads(*g_benchmark_job);
    g_benchmark_job.reset();
}

void start_benchmark_job_locked(const std::string& name) {
    stop_active_benchmark_job_locked();

    auto job = std::make_unique<BenchmarkJob>();
    job->name = name;
    job->kotlin_score = benchmark_kotlin_score(name);
    BenchmarkJob* raw = job.get();

    const int workload_units = benchmark_workload_units(name);
    const int iterations_per_unit = 200000;
    auto worker_fn = [raw, workload_units, iterations_per_unit](std::atomic<int>& progress_slot) {
        volatile std::uint64_t sink = 0;
        for (int unit = 1; unit <= workload_units; ++unit) {
            if (raw->cancel.load()) break;
            for (int i = 0; i < iterations_per_unit; ++i) {
                sink += static_cast<std::uint64_t>((i * 1664525u + 1013904223u) ^ unit);
            }
            progress_slot.store((unit * 100) / workload_units);
        }
        (void)sink;
    };

    raw->worker_a = std::thread(worker_fn, std::ref(raw->worker_a_progress));
    raw->worker_b = std::thread(worker_fn, std::ref(raw->worker_b_progress));

    raw->monitor = std::thread([raw]() {
        const auto t0 = std::chrono::steady_clock::now();
        enqueue_benchmark_event("progress", benchmark_progress_payload(raw->name, 0, "starting"));
        int last_percent = -1;
        while (!raw->cancel.load()) {
            const bool a_done = raw->worker_a_progress.load() >= 100;
            const bool b_done = raw->worker_b_progress.load() >= 100;
            const int percent = (raw->worker_a_progress.load() + raw->worker_b_progress.load()) / 2;
            if (percent != last_percent) {
                enqueue_benchmark_event("progress", benchmark_progress_payload(raw->name, percent, "running"));
                last_percent = percent;
            }
            if (a_done && b_done) break;
            std::this_thread::sleep_for(std::chrono::milliseconds(40));
        }
        if (raw->worker_a.joinable()) raw->worker_a.join();
        if (raw->worker_b.joinable()) raw->worker_b.join();

        if (raw->cancel.load()) {
            enqueue_benchmark_event("completed", benchmark_completed_payload(
                raw->name, 0, 0, raw->kotlin_score, "cancelled"));
            raw->done.store(true);
            return;
        }

        const auto t1 = std::chrono::steady_clock::now();
        raw->duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
        if (raw->duration_ms <= 0) raw->duration_ms = 1;

        const std::int64_t total_ops = static_cast<std::int64_t>(2) * benchmark_workload_units(raw->name) * iterations_per_unit;
        raw->sms_score = static_cast<int>((total_ops * 1000) / raw->duration_ms);
        const std::string summary = "done in " + std::to_string(raw->duration_ms) + " ms";
        enqueue_benchmark_event("progress", benchmark_progress_payload(raw->name, 100, "finalizing"));
        enqueue_benchmark_event("completed", benchmark_completed_payload(
            raw->name, raw->duration_ms, raw->sms_score, raw->kotlin_score, summary));
        raw->done.store(true);
    });

    g_benchmark_job = std::move(job);
}
} // namespace

void reset_benchmark_runtime() {
    std::lock_guard<std::mutex> lock(g_benchmark_mutex);
    stop_active_benchmark_job_locked();
    std::lock_guard<std::mutex> qlock(g_benchmark_events_mutex);
    g_benchmark_events.clear();
}

bool pop_benchmark_event(std::string& out_event_name, std::string& out_payload_json) {
    std::lock_guard<std::mutex> lock(g_benchmark_events_mutex);
    if (g_benchmark_events.empty()) return false;
    out_event_name = g_benchmark_events.front().first;
    out_payload_json = g_benchmark_events.front().second;
    g_benchmark_events.pop_front();
    return true;
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static std::string json_string(const std::string& s) {
    std::string out = "\"";
    for (unsigned char c : s) {
        if      (c == '"')  out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else if (c == '\n') out += "\\n";
        else if (c == '\r') out += "\\r";
        else if (c == '\t') out += "\\t";
        else                out += static_cast<char>(c);
    }
    out += "\"";
    return out;
}

static void write_out(char* buf, int cap, const std::string& s) {
    if (buf == nullptr || cap <= 0) return;
    std::snprintf(buf, static_cast<std::size_t>(cap), "%s", s.c_str());
}

static std::string variant_to_json(const Variant& value) {
    if (value.get_type() == Variant::NIL) return "null";
    const String json = JSON::stringify(value);
    return json.utf8().get_data();
}

static Variant parse_json_variant(const std::string& json_value, bool* ok = nullptr) {
    const Variant parsed = JSON::parse_string(String(json_value.c_str()));
    const bool parsed_ok = !(parsed.get_type() == Variant::NIL && json_value != "null");
    if (ok) *ok = parsed_ok;
    return parsed_ok ? parsed : Variant();
}

static std::unordered_map<std::int64_t, TreeItem*>& tree_item_handles() {
    static std::unordered_map<std::int64_t, TreeItem*> handles;
    return handles;
}

static std::int64_t& next_tree_item_handle() {
    static std::int64_t next = 1;
    return next;
}

static std::int64_t register_tree_item_handle(TreeItem* item) {
    if (item == nullptr) return 0;
    const std::int64_t handle = next_tree_item_handle()++;
    tree_item_handles()[handle] = item;
    return handle;
}

static String app_url_base_dir() {
    const char* env = std::getenv("FORGE_RUNNER_URL");
    if (env == nullptr || env[0] == '\0') return String();
    std::string url(env);
    if (url.rfind("file://", 0) != 0) return String();
    std::string path = url.substr(7);
    return String(fs::path(path).parent_path().string().c_str());
}

static String appres_root_dir() {
    const char* appres = std::getenv("FORGE_RUNNER_APPRES_ROOT");
    if (appres == nullptr || appres[0] == '\0') return String();
    return String(appres);
}

static Ref<Texture2D> load_texture_best_effort(const String& raw_path) {
    if (raw_path.is_empty()) return Ref<Texture2D>();
    const String base_dir = app_url_base_dir();
    const String appres_root = appres_root_dir();
    const std::string resolved = forge::resolve_runtime_asset_path(
        raw_path.utf8().get_data(),
        base_dir.utf8().get_data(),
        appres_root.utf8().get_data());
    if (resolved.empty()) return Ref<Texture2D>();

    const String resolved_path(resolved.c_str());
    if (!FileAccess::file_exists(resolved_path)) {
        static std::unordered_set<std::string> warned_missing;
        if (warned_missing.insert(resolved).second) {
            UtilityFunctions::push_warning(String("[ForgeRunner.Native] Missing tree icon: ") + resolved_path);
        }
        return Ref<Texture2D>();
    }

    Ref<Image> img;
    img.instantiate();
    if (img.is_null() || img->load(resolved_path) != OK) {
        static std::unordered_set<std::string> warned_unreadable;
        if (warned_unreadable.insert(resolved).second) {
            UtilityFunctions::push_warning(String("[ForgeRunner.Native] Failed to read tree icon: ") + resolved_path);
        }
        return Ref<Texture2D>();
    }
    Ref<ImageTexture> tex;
    tex.instantiate();
    if (tex.is_null()) return Ref<Texture2D>();
    tex->set_image(img);
    Ref<Texture2D> out = tex;
    return out;
}

// Strip surrounding double-quotes from a JSON string value.
static std::string json_unquote(const std::string& v) {
    if (v.size() >= 2 && v.front() == '"' && v.back() == '"')
        return v.substr(1, v.size() - 2);
    return v;
}

// Extract first string argument from a JSON array: ["arg1", ...]
static std::string first_string_arg(const std::string& args_json) {
    const auto start = args_json.find('"');
    if (start == std::string::npos) return {};
    std::size_t pos = start + 1;
    while (pos < args_json.size()) {
        if (args_json[pos] == '"' && args_json[pos - 1] != '\\')
            return args_json.substr(start + 1, pos - start - 1);
        ++pos;
    }
    return {};
}

static LineEdit* resolve_line_edit_by_id(const std::string& id) {
    auto it = SmsBridge::id_map().find(id);
    if (it == SmsBridge::id_map().end() || it->second == nullptr) return nullptr;
    return Object::cast_to<LineEdit>(it->second);
}

struct NumericLineEditConfig {
    std::string axis = "x";
    std::string unit;
    int decimals = 3;
    Color axis_color = Color(1.0f, 1.0f, 1.0f, 1.0f);
};

static std::unordered_map<std::string, NumericLineEditConfig>& numeric_line_edit_configs() {
    static std::unordered_map<std::string, NumericLineEditConfig> configs;
    return configs;
}

static double parse_numeric_from_text(const String& text) {
    const std::string raw = text.utf8().get_data();
    std::string normalized;
    normalized.reserve(raw.size());
    for (unsigned char c : raw) {
        const bool ok =
            (c >= '0' && c <= '9') || c == '.' || c == ',' ||
            c == '-' || c == '+' || c == 'e' || c == 'E';
        normalized += ok ? static_cast<char>(c == ',' ? '.' : c) : ' ';
    }
    std::stringstream ss(normalized);
    double v = 0.0;
    ss >> v;
    return ss.fail() ? 0.0 : v;
}

static std::string format_numeric_preview(const NumericLineEditConfig& cfg, double value) {
    std::ostringstream ss;
    ss.setf(std::ios::fixed, std::ios::floatfield);
    ss << std::setprecision(CLAMP(cfg.decimals, 0, 6)) << value;
    const std::string number = ss.str();
    if (cfg.unit.empty()) {
        return cfg.axis + " " + number;
    }
    return cfg.axis + " " + number + " " + cfg.unit;
}

// ---------------------------------------------------------------------------
// Platform library helpers
// ---------------------------------------------------------------------------

static void* platform_load_lib(const fs::path& p) {
#if defined(_WIN32)
    return LoadLibraryW(p.wstring().c_str());
#else
    // RTLD_GLOBAL: make sms_native's symbols (sms_native_llvm_*) available
    // to subsequently loaded compiled SMS shared libraries.
    return dlopen(p.c_str(), RTLD_NOW | RTLD_GLOBAL);
#endif
}

static void* platform_load_sym(void* handle, const char* name) {
    if (!handle) return nullptr;
#if defined(_WIN32)
    return reinterpret_cast<void*>(GetProcAddress(static_cast<HMODULE>(handle), name));
#else
    return dlsym(handle, name);
#endif
}

static void platform_free_lib(void* handle) {
    if (!handle) return;
#if defined(_WIN32)
    FreeLibrary(static_cast<HMODULE>(handle));
#else
    dlclose(handle);
#endif
}

static std::string lib_extension() {
#if defined(_WIN32)
    return ".dll";
#elif defined(__APPLE__)
    return ".dylib";
#else
    return ".so";
#endif
}

// ---------------------------------------------------------------------------
// SMS UI callbacks (C-linkage wrappers called by the SMS interpreter)
// ---------------------------------------------------------------------------

static int sms_ui_get(
    const char* object_id, const char* property,
    char* out_json, int out_cap, char*, int)
{
    const std::string id   = object_id ? object_id : "";
    const std::string prop = property  ? property  : "";

    if (prop == "__exists") {
        const auto it = SmsBridge::id_map().find(id);
        const bool exists = (it != SmsBridge::id_map().end() && it->second != nullptr);
        write_out(out_json, out_cap, exists ? "1" : "0");
        return 0;
    }

    auto it = SmsBridge::id_map().find(id);
    if (it == SmsBridge::id_map().end() || it->second == nullptr) {
        write_out(out_json, out_cap, "null");
        return 0;
    }
    Control* ctrl = it->second;
    std::string result = "null";

    if (prop == "visible") {
        result = ctrl->is_visible() ? "true" : "false";
    } else if (prop == "text") {
        String t;
        if      (auto* l   = Object::cast_to<Label>(ctrl))         t = l->get_text();
        else if (auto* b   = Object::cast_to<Button>(ctrl))        t = b->get_text();
        else if (auto* le  = Object::cast_to<LineEdit>(ctrl))      t = le->get_text();
        else if (auto* te  = Object::cast_to<TextEdit>(ctrl))      t = te->get_text();
        else if (auto* rtl = Object::cast_to<RichTextLabel>(ctrl)) t = rtl->get_text();
        result = json_string(t.utf8().get_data());
    } else if (prop == "value") {
        double v = 0.0;
        if      (auto* sb = Object::cast_to<SpinBox>(ctrl))       v = sb->get_value();
        else if (auto* sl = Object::cast_to<Slider>(ctrl))        v = sl->get_value();
        else if (auto* pb = Object::cast_to<ProgressBar>(ctrl))   v = pb->get_value();
        char buf[64]; std::snprintf(buf, sizeof(buf), "%g", v);
        result = buf;
    } else if (prop == "checked" || prop == "buttonPressed") {
        bool pressed = false;
        if (auto* btn = Object::cast_to<BaseButton>(ctrl)) pressed = btn->is_pressed();
        result = pressed ? "true" : "false";
    } else if (prop == "disabled") {
        bool dis = false;
        if (auto* btn = Object::cast_to<BaseButton>(ctrl)) dis = btn->is_disabled();
        result = dis ? "true" : "false";
    } else if (prop == "selectedIndex") {
        int idx = -1;
        if (auto* ob = Object::cast_to<OptionButton>(ctrl)) idx = ob->get_selected();
        char buf[32]; std::snprintf(buf, sizeof(buf), "%d", idx);
        result = buf;
    } else if (prop == "selectedText") {
        std::string t;
        if (auto* ob = Object::cast_to<OptionButton>(ctrl)) {
            const int sel = ob->get_selected();
            if (sel >= 0) t = ob->get_item_text(sel).utf8().get_data();
        }
        result = json_string(t);
    } else if (prop == "caretColumn") {
        int col = 0;
        if (auto* le = Object::cast_to<LineEdit>(ctrl)) col = le->get_caret_column();
        char buf[32]; std::snprintf(buf, sizeof(buf), "%d", col);
        result = buf;
    } else if (prop == "scrollV") {
        int v = 0;
        if (auto* sc = Object::cast_to<ScrollContainer>(ctrl)) v = sc->get_v_scroll();
        char buf[32]; std::snprintf(buf, sizeof(buf), "%d", v);
        result = buf;
    } else {
        const Variant value = ctrl->get(StringName(prop.c_str()));
        result = variant_to_json(value);
    }

    write_out(out_json, out_cap, result);
    return 0;
}

static int sms_ui_set(
    const char* object_id, const char* property, const char* value_json,
    char*, int)
{
    const std::string id    = object_id  ? object_id  : "";
    const std::string prop  = property   ? property   : "";
    const std::string value = value_json ? value_json : "null";

    auto it = SmsBridge::id_map().find(id);
    if (it == SmsBridge::id_map().end() || it->second == nullptr) return 0;
    Control* ctrl = it->second;

    if (prop == "visible") {
        ctrl->set_visible(value == "true" || value == "1");
    } else if (prop == "text") {
        const std::string decoded = forge::decode_json_string_or_fallback(value);
        const String t(decoded.c_str());
        if      (auto* l   = Object::cast_to<Label>(ctrl))         l->set_text(t);
        else if (auto* b   = Object::cast_to<Button>(ctrl))        b->set_text(t);
        else if (auto* le  = Object::cast_to<LineEdit>(ctrl))      le->set_text(t);
        else if (auto* te  = Object::cast_to<TextEdit>(ctrl))      te->set_text(t);
        else if (auto* rtl = Object::cast_to<RichTextLabel>(ctrl)) rtl->set_text(t);
    } else if (prop == "value") {
        double v = 0.0;
        try { v = std::stod(value); } catch (...) {}
        if      (auto* sb = Object::cast_to<SpinBox>(ctrl))       sb->set_value(v);
        else if (auto* sl = Object::cast_to<Slider>(ctrl))        sl->set_value(v);
        else if (auto* pb = Object::cast_to<ProgressBar>(ctrl))   pb->set_value(v);
    } else if (prop == "checked" || prop == "buttonPressed") {
        if (auto* btn = Object::cast_to<BaseButton>(ctrl))
            btn->set_pressed(value == "true" || value == "1");
    } else if (prop == "disabled") {
        if (auto* btn = Object::cast_to<BaseButton>(ctrl))
            btn->set_disabled(value == "true" || value == "1");
    } else if (prop == "selectedIndex") {
        int idx = 0;
        try { idx = std::stoi(value); } catch (...) {}
        if (auto* ob = Object::cast_to<OptionButton>(ctrl)) ob->select(idx);
    } else if (prop == "scrollV") {
        int v = 0;
        try { v = std::stoi(value); } catch (...) {}
        if (auto* sc = Object::cast_to<ScrollContainer>(ctrl)) sc->set_v_scroll(v);
    } else {
        bool parsed_ok = false;
        Variant parsed_value = parse_json_variant(value, &parsed_ok);
        if (!parsed_ok) {
            parsed_value = String(json_unquote(value).c_str());
        }
        ctrl->set(StringName(prop.c_str()), parsed_value);
    }
    return 0;
}

static int sms_ui_get_string(
    const char* object_id, const char* property,
    char* out_text, int out_cap, char* error, int error_cap)
{
    const std::string id   = object_id ? object_id : "";
    const std::string prop = property  ? property  : "";
    auto it = SmsBridge::id_map().find(id);
    if (it == SmsBridge::id_map().end() || it->second == nullptr) {
        write_out(error, error_cap, "ui object not found");
        return 1;
    }
    if (prop != "text") {
        write_out(error, error_cap, "string bridge supports only 'text'");
        return 1;
    }

    Control* ctrl = it->second;
    String t;
    if      (auto* l   = Object::cast_to<Label>(ctrl))         t = l->get_text();
    else if (auto* b   = Object::cast_to<Button>(ctrl))        t = b->get_text();
    else if (auto* le  = Object::cast_to<LineEdit>(ctrl))      t = le->get_text();
    else if (auto* te  = Object::cast_to<TextEdit>(ctrl))      t = te->get_text();
    else if (auto* rtl = Object::cast_to<RichTextLabel>(ctrl)) t = rtl->get_text();
    else {
        write_out(error, error_cap, "ui object does not support text");
        return 1;
    }
    write_out(out_text, out_cap, t.utf8().get_data());
    return 0;
}

static int sms_ui_set_string(
    const char* object_id, const char* property, const char* value_text,
    char* error, int error_cap)
{
    const std::string id   = object_id ? object_id : "";
    const std::string prop = property  ? property  : "";
    auto it = SmsBridge::id_map().find(id);
    if (it == SmsBridge::id_map().end() || it->second == nullptr) {
        write_out(error, error_cap, "ui object not found");
        return 1;
    }
    if (prop != "text") {
        write_out(error, error_cap, "string bridge supports only 'text'");
        return 1;
    }

    const String t((value_text ? value_text : ""));
    Control* ctrl = it->second;
    if      (auto* l   = Object::cast_to<Label>(ctrl))         l->set_text(t);
    else if (auto* b   = Object::cast_to<Button>(ctrl))        b->set_text(t);
    else if (auto* le  = Object::cast_to<LineEdit>(ctrl))      le->set_text(t);
    else if (auto* te  = Object::cast_to<TextEdit>(ctrl))      te->set_text(t);
    else if (auto* rtl = Object::cast_to<RichTextLabel>(ctrl)) rtl->set_text(t);
    else {
        write_out(error, error_cap, "ui object does not support text");
        return 1;
    }
    return 0;
}

static int sms_ui_invoke(
    const char* object_id, const char* method, const char* args_json,
    char* out_json, int out_cap, char*, int)
{
    const std::string id    = object_id ? object_id : "";
    const std::string mname = method    ? method    : "";
    const std::string args  = args_json ? args_json : "[]";

    if (id == "__log__") {
        std::string msg = first_string_arg(args);
        if (msg.empty()) {
            bool parsed_ok = false;
            const Variant parsed = parse_json_variant(args, &parsed_ok);
            if (parsed_ok && parsed.get_type() == Variant::ARRAY) {
                const Array arr = static_cast<Array>(parsed);
                if (!arr.is_empty()) {
                    msg = variant_to_json(arr[0]);
                }
            }
            if (msg.empty()) {
                msg = args;
            }
        }

        const std::string level = mname.empty() ? "info" : mname;
        const std::string line = "[SMS][" + level + "] " + msg;
        if (mname == "error") {
            UtilityFunctions::printerr(String(line.c_str()));
        } else if (mname == "warn" || mname == "warning") {
            UtilityFunctions::push_warning(String(line.c_str()));
        } else {
            UtilityFunctions::print(String(line.c_str()));
        }
        write_out(out_json, out_cap, "null");
        return 0;
    }

    if (id == "__fs__") {
        bool parsed_ok = false;
        const Variant parsed = parse_json_variant(args, &parsed_ok);
        const Array arr = (parsed_ok && parsed.get_type() == Variant::ARRAY) ? static_cast<Array>(parsed) : Array();
        auto arg_string = [&](int idx) -> std::string {
            if (idx < 0 || idx >= arr.size()) return {};
            return static_cast<String>(arr[idx]).utf8().get_data();
        };

        auto resolve_fs_path = [&](const std::string& raw) -> std::string {
            if (raw.empty()) return {};
            if (raw.rfind("user:/", 0) == 0) {
                String user_uri = String("user://") + String(raw.substr(6).c_str());
                if (ProjectSettings::get_singleton() != nullptr) {
                    return std::string(ProjectSettings::get_singleton()->globalize_path(user_uri).utf8().get_data());
                }
            }
            return forge::resolve_runtime_asset_path(
                raw,
                app_url_base_dir().utf8().get_data(),
                appres_root_dir().utf8().get_data());
        };

        auto join_display_path = [&](const std::string& base, const std::string& name) -> std::string {
            const bool is_uri = base.rfind("res:/", 0) == 0
                || base.rfind("appRes:/", 0) == 0
                || base.rfind("user:/", 0) == 0;
            if (is_uri) {
                if (base == "res:/" || base == "appRes:/" || base == "user:/") {
                    return base + name;
                }
                return base + "/" + name;
            }
            return (fs::path(base) / name).string();
        };

        if (mname == "exists" && arr.size() >= 1) {
            const std::string resolved = resolve_fs_path(arg_string(0));
            const bool exists = !resolved.empty() && fs::exists(fs::path(resolved));
            write_out(out_json, out_cap, exists ? "true" : "false");
            return 0;
        }

        if (mname == "readText" && arr.size() >= 1) {
            const std::string resolved = resolve_fs_path(arg_string(0));
            std::ifstream in(resolved, std::ios::binary);
            std::string content;
            if (in.is_open()) {
                std::ostringstream ss;
                ss << in.rdbuf();
                content = ss.str();
            }
            write_out(out_json, out_cap, json_string(content));
            return 0;
        }

        if (mname == "writeText" && arr.size() >= 2) {
            const std::string resolved = resolve_fs_path(arg_string(0));
            bool ok = false;
            if (!resolved.empty()) {
                std::error_code ec;
                fs::create_directories(fs::path(resolved).parent_path(), ec);
                std::ofstream out(resolved, std::ios::binary | std::ios::trunc);
                if (out.is_open()) {
                    out << arg_string(1);
                    ok = static_cast<bool>(out);
                }
            }
            write_out(out_json, out_cap, ok ? "true" : "false");
            return 0;
        }

        if (mname == "list" && arr.size() >= 1) {
            const std::string base_input = arg_string(0);
            const std::string resolved = resolve_fs_path(base_input);
            std::error_code ec;
            std::ostringstream json;
            json << "[";
            bool first = true;
            if (!resolved.empty()) {
                for (const auto& entry : fs::directory_iterator(fs::path(resolved), ec)) {
                    if (ec) break;
                    const std::string name = entry.path().filename().string();
                    const std::string child_path = join_display_path(base_input, name);
                    const bool is_dir = entry.is_directory(ec);
                    if (!first) json << ",";
                    first = false;
                    json << "{"
                         << "\"Name\":" << json_string(name) << ","
                         << "\"Path\":" << json_string(child_path) << ","
                         << "\"IsDirectory\":" << (is_dir ? "true" : "false")
                         << "}";
                }
            }
            json << "]";
            write_out(out_json, out_cap, json.str());
            return 0;
        }

        write_out(out_json, out_cap, "null");
        return 0;
    }

    if (id == "__os__") {
        bool parsed_ok = false;
        const Variant parsed = parse_json_variant(args, &parsed_ok);
        const Array arr = (parsed_ok && parsed.get_type() == Variant::ARRAY) ? static_cast<Array>(parsed) : Array();
        auto arg_string = [&](int idx) -> std::string {
            if (idx < 0 || idx >= arr.size()) return {};
            return static_cast<String>(arr[idx]).utf8().get_data();
        };

        auto resolve_os_path = [&](const std::string& raw) -> std::string {
            if (raw.empty()) return {};
            if (raw.rfind("user:/", 0) == 0) {
                String user_uri = String("user://") + String(raw.substr(6).c_str());
                if (ProjectSettings::get_singleton() != nullptr) {
                    return std::string(ProjectSettings::get_singleton()->globalize_path(user_uri).utf8().get_data());
                }
            }
            return forge::resolve_runtime_asset_path(
                raw,
                app_url_base_dir().utf8().get_data(),
                appres_root_dir().utf8().get_data());
        };

        if (mname == "smsRuntimeMode" || mname == "runtimeMode") {
            write_out(out_json, out_cap, json_string(current_sms_runtime_mode_name()));
            return 0;
        }

        if (mname == "quit" || mname == "exit") {
            auto* main_loop = Engine::get_singleton() ? Engine::get_singleton()->get_main_loop() : nullptr;
            auto* tree = Object::cast_to<SceneTree>(main_loop);
            if (tree != nullptr) {
                tree->quit(0);
            } else if (main_loop != nullptr && main_loop->has_method(StringName("quit"))) {
                main_loop->call("quit", 0);
            } else {
                UtilityFunctions::push_warning("[ForgeRunner.Native][SMS] os.quit requested but SceneTree is unavailable.");
            }
            write_out(out_json, out_cap, "null");
            return 0;
        }

        if (mname == "fileExists" && arr.size() >= 1) {
            const std::string resolved = resolve_os_path(arg_string(0));
            const bool exists = !resolved.empty() && fs::exists(fs::path(resolved));
            write_out(out_json, out_cap, exists ? "true" : "false");
            return 0;
        }

        write_out(out_json, out_cap, "null");
        return 0;
    }

    if (id == "__benchmark__") {
        bool parsed_ok = false;
        const Variant parsed = parse_json_variant(args, &parsed_ok);
        const Array arr = (parsed_ok && parsed.get_type() == Variant::ARRAY) ? static_cast<Array>(parsed) : Array();
        auto arg_string = [&](int idx) -> std::string {
            if (idx < 0 || idx >= arr.size()) return {};
            return static_cast<String>(arr[idx]).utf8().get_data();
        };

        if (mname == "start" && arr.size() >= 1) {
            const std::string bench_name = arg_string(0);
            if (bench_name.empty()) {
                write_out(out_json, out_cap, "false");
                return 0;
            }
            {
                std::lock_guard<std::mutex> lock(g_benchmark_mutex);
                start_benchmark_job_locked(bench_name);
            }
            write_out(out_json, out_cap, "true");
            return 0;
        }
        if (mname == "cancel") {
            std::lock_guard<std::mutex> lock(g_benchmark_mutex);
            stop_active_benchmark_job_locked();
            write_out(out_json, out_cap, "true");
            return 0;
        }

        write_out(out_json, out_cap, "null");
        return 0;
    }

    if (id == "__ui__" || id == "ui") {
        bool parsed_ok = false;
        const Variant parsed = parse_json_variant(args, &parsed_ok);
        const Array arr = (parsed_ok && parsed.get_type() == Variant::ARRAY) ? static_cast<Array>(parsed) : Array();
        auto arg_string = [&](int idx) -> std::string {
            if (idx < 0 || idx >= arr.size()) return {};
            return static_cast<String>(arr[idx]).utf8().get_data();
        };
        static std::string s_last_project_path;

        if (mname == "getObject") {
            write_out(out_json, out_cap, json_string(arg_string(0)));
            return 0;
        }
        if (mname == "setLastProjectPath") {
            s_last_project_path = arg_string(0);
            write_out(out_json, out_cap, "null");
            return 0;
        }
        if (mname == "getLastProjectPath") {
            write_out(out_json, out_cap, json_string(s_last_project_path));
            return 0;
        }
        if (mname == "hasLastProject") {
            write_out(out_json, out_cap, s_last_project_path.empty() ? "false" : "true");
            return 0;
        }
        if (mname == "openFileDialog" || mname == "openSaveFileDialog") {
            const auto cb = arg_string(0);
            const auto filter = arg_string(1);
            if (auto hook = ui_open_dialog_hook()) {
                hook(cb, filter, mname == "openSaveFileDialog");
            } else {
                UtilityFunctions::push_warning("[ForgeRunner.Native] ui.open*FileDialog hook is not set.");
            }
            write_out(out_json, out_cap, "null");
            return 0;
        }
        if (mname == "copyTemplateFilesToProject") {
            write_out(out_json, out_cap, "true");
            return 0;
        }
        if (mname == "configureNumericLineEdit") {
            const std::string control_id = arg_string(0);
            if (LineEdit* le = resolve_line_edit_by_id(control_id)) {
                const std::string axis = arg_string(1);
                const std::string unit = arg_string(2);
                const std::string color_raw = arg_string(3);
                double step = 0.01;
                if (arr.size() > 4) {
                    const Variant step_raw = arr[4];
                    if (step_raw.get_type() == Variant::INT || step_raw.get_type() == Variant::FLOAT) {
                        step = static_cast<double>(step_raw);
                    }
                }
                double drag_sensitivity = 0.02;
                if (arr.size() > 5) {
                    const Variant drag_raw = arr[5];
                    if (drag_raw.get_type() == Variant::INT || drag_raw.get_type() == Variant::FLOAT) {
                        drag_sensitivity = static_cast<double>(drag_raw);
                    }
                }
                int decimals = 3;
                if (arr.size() > 6) {
                    const Variant decimals_raw = arr[6];
                    if (decimals_raw.get_type() == Variant::INT || decimals_raw.get_type() == Variant::FLOAT) {
                        decimals = CLAMP(static_cast<int>(decimals_raw), 0, 6);
                    }
                }
                const Color color = color_raw.empty()
                    ? Color(1.0f, 1.0f, 1.0f, 1.0f)
                    : Color(String(color_raw.c_str()));
                if (le->has_method("set_numeric_config")) {
                    le->call("set_numeric_config", String(axis.c_str()), String(unit.c_str()), color, step, drag_sensitivity, decimals);
                    write_out(out_json, out_cap, "true");
                    return 0;
                }

                NumericLineEditConfig cfg;
                cfg.axis = axis;
                if (cfg.axis.empty()) cfg.axis = "x";
                std::transform(cfg.axis.begin(), cfg.axis.end(), cfg.axis.begin(), [](unsigned char c) {
                    return static_cast<char>(std::tolower(c));
                });
                cfg.unit = unit;
                if (!color_raw.empty()) {
                    cfg.axis_color = color;
                }
                cfg.decimals = decimals;
                numeric_line_edit_configs()[control_id] = cfg;

                le->set_horizontal_alignment(HORIZONTAL_ALIGNMENT_RIGHT);
                le->set_select_all_on_focus(true);
                le->add_theme_color_override("font_color", cfg.axis_color);
                le->add_theme_color_override("caret_color", cfg.axis_color);

                const double current_value = parse_numeric_from_text(le->get_text());
                le->set_text(String(format_numeric_preview(cfg, current_value).c_str()));
            }
            write_out(out_json, out_cap, "true");
            return 0;
        }
        if (mname == "setNumericLineEditValue") {
            const std::string control_id = arg_string(0);
            if (LineEdit* le = resolve_line_edit_by_id(control_id)) {
                double numeric_value = 0.0;
                if (arr.size() > 1) {
                    const Variant value = arr[1];
                    if (value.get_type() == Variant::FLOAT || value.get_type() == Variant::INT) {
                        numeric_value = static_cast<double>(value);
                    } else {
                        numeric_value = parse_numeric_from_text(String(value));
                    }
                }
                if (le->has_method("set_numeric_value")) {
                    le->call("set_numeric_value", numeric_value);
                    write_out(out_json, out_cap, "true");
                    return 0;
                }
                auto cfg_it = numeric_line_edit_configs().find(control_id);
                if (cfg_it != numeric_line_edit_configs().end()) {
                    le->set_text(String(format_numeric_preview(cfg_it->second, numeric_value).c_str()));
                } else {
                    le->set_text(String::num(numeric_value));
                }
            }
            write_out(out_json, out_cap, "true");
            return 0;
        }
        if (mname == "getNumericLineEditValue") {
            const std::string control_id = arg_string(0);
            double value = 0.0;
            if (LineEdit* le = resolve_line_edit_by_id(control_id)) {
                if (le->has_method("get_numeric_value")) {
                    const Variant v = le->call("get_numeric_value");
                    if (v.get_type() == Variant::FLOAT || v.get_type() == Variant::INT) {
                        value = static_cast<double>(v);
                    }
                } else {
                    value = parse_numeric_from_text(le->get_text());
                }
            }
            write_out(out_json, out_cap, std::to_string(value));
            return 0;
        }
        write_out(out_json, out_cap, "null");
        return 0;
    }

    auto it = SmsBridge::id_map().find(id);
    if (it == SmsBridge::id_map().end() || it->second == nullptr) {
        write_out(out_json, out_cap, "null");
        return 0;
    }
    Control* ctrl = it->second;

    if (mname == "focus") {
        ctrl->grab_focus();
    } else if (mname == "scrollToBottom") {
        if (auto* sc = Object::cast_to<ScrollContainer>(ctrl))
            sc->set_v_scroll(std::numeric_limits<int>::max());
    } else if (auto* tree = Object::cast_to<Tree>(ctrl)) {
        if (mname == "Clear" || mname == "clear") {
            tree->clear();
            tree_item_handles().clear();
            write_out(out_json, out_cap, "null");
            return 0;
        }
        if (mname == "CreateRoot") {
            bool parsed_ok = false;
            const Variant parsed = parse_json_variant(args, &parsed_ok);
            if (!parsed_ok || parsed.get_type() != Variant::ARRAY) {
                write_out(out_json, out_cap, "0");
                return 0;
            }
            const Array arr = parsed;
            const String text = arr.size() > 0 ? static_cast<String>(arr[0]) : String();
            const String path = arr.size() > 1 ? static_cast<String>(arr[1]) : String();

            TreeItem* item = tree->create_item();
            if (item != nullptr) {
                item->set_text(0, text);
                item->set_collapsed(false);
                item->set_metadata(0, path);
            }
            const std::int64_t handle = register_tree_item_handle(item);
            write_out(out_json, out_cap, std::to_string(static_cast<long long>(handle)));
            return 0;
        }
        if (mname == "CreateChild") {
            bool parsed_ok = false;
            const Variant parsed = parse_json_variant(args, &parsed_ok);
            if (!parsed_ok || parsed.get_type() != Variant::ARRAY) {
                write_out(out_json, out_cap, "0");
                return 0;
            }
            const Array arr = parsed;
            if (arr.size() < 4) {
                write_out(out_json, out_cap, "0");
                return 0;
            }

            const std::int64_t parent_handle = static_cast<std::int64_t>(arr[0]);
            auto parent_it = tree_item_handles().find(parent_handle);
            if (parent_it == tree_item_handles().end() || parent_it->second == nullptr) {
                write_out(out_json, out_cap, "0");
                return 0;
            }

            const String text = static_cast<String>(arr[1]);
            const String path = static_cast<String>(arr[2]);
            const bool is_directory = static_cast<bool>(arr[3]);

            TreeItem* item = tree->create_item(parent_it->second);
            if (item != nullptr) {
                item->set_text(0, is_directory ? (text + String("/")) : text);
                item->set_metadata(0, path);
                item->set_collapsed(true);
            }
            const std::int64_t handle = register_tree_item_handle(item);
            write_out(out_json, out_cap, std::to_string(static_cast<long long>(handle)));
            return 0;
        }
        if (mname == "AddButton") {
            bool parsed_ok = false;
            const Variant parsed = parse_json_variant(args, &parsed_ok);
            if (!parsed_ok || parsed.get_type() != Variant::ARRAY) {
                write_out(out_json, out_cap, "false");
                return 0;
            }
            const Array arr = parsed;
            if (arr.size() < 3) {
                write_out(out_json, out_cap, "false");
                return 0;
            }

            const std::int64_t item_handle = static_cast<std::int64_t>(arr[0]);
            auto item_it = tree_item_handles().find(item_handle);
            if (item_it == tree_item_handles().end() || item_it->second == nullptr) {
                write_out(out_json, out_cap, "false");
                return 0;
            }

            const String icon_path = static_cast<String>(arr[1]);
            const int button_id = static_cast<int>(arr[2]);
            const String tooltip = arr.size() > 3 ? static_cast<String>(arr[3]) : String();

            Ref<Texture2D> icon = load_texture_best_effort(icon_path);
            if (icon.is_null()) {
                write_out(out_json, out_cap, "false");
                return 0;
            }

            item_it->second->add_button(0, icon, button_id, false, tooltip);
            write_out(out_json, out_cap, "true");
            return 0;
        }
        if (mname == "GetSelectedPath") {
            TreeItem* selected = tree->get_selected();
            const String path = selected != nullptr ? static_cast<String>(selected->get_metadata(0)) : String();
            write_out(out_json, out_cap, variant_to_json(path));
            return 0;
        }
        if (mname == "BindEvents") {
            // Events are wired by the native runner bootstrap where applicable.
            write_out(out_json, out_cap, "null");
            return 0;
        }
    } else if (mname == "clearItems") {
        if      (auto* ob = Object::cast_to<OptionButton>(ctrl)) ob->clear();
        else if (auto* il = Object::cast_to<ItemList>(ctrl))     il->clear();
    } else if (mname == "addItem") {
        const String item(first_string_arg(args).c_str());
        if      (auto* ob = Object::cast_to<OptionButton>(ctrl)) ob->add_item(item);
        else if (auto* il = Object::cast_to<ItemList>(ctrl))     il->add_item(item);
    } else if (ctrl->has_method(StringName(mname.c_str()))) {
        Array call_args;
        bool parsed_ok = false;
        const Variant parsed = parse_json_variant(args, &parsed_ok);
        if (parsed_ok && parsed.get_type() == Variant::ARRAY) {
            call_args = parsed;
        } else if (parsed_ok && parsed.get_type() != Variant::NIL) {
            call_args.push_back(parsed);
        }
        const Variant ret = ctrl->callv(StringName(mname.c_str()), call_args);
        write_out(out_json, out_cap, variant_to_json(ret));
        return 0;
    }

    write_out(out_json, out_cap, "null");
    return 0;
}

// ---------------------------------------------------------------------------
// SmsBridge implementation
// ---------------------------------------------------------------------------

SmsBridge::SmsBridge() = default;

SmsBridge::~SmsBridge() {
    unload();
}

bool SmsBridge::load(const std::string& repo_root) {
    if (loaded_) return true;

    const char* env_dir = std::getenv("SMS_NATIVE_LIB_DIR");
    const fs::path lib_dir = (env_dir && env_dir[0] != '\0')
        ? fs::path(env_dir)
        : fs::path(repo_root) / "ForgeRunner.Native" / "build";
    const fs::path lib_path = lib_dir / ("libsms_native" + lib_extension());

    lib_handle_ = platform_load_lib(lib_path);
#if defined(__ANDROID__)
    if (!lib_handle_) {
        // In packaged Android builds, try direct soname first.
        lib_handle_ = platform_load_lib("libsms_native.so");
    }
    if (!lib_handle_) {
        // Final fallback for Android: SMS runtime is linked into forge_runner_native.
        create_fn_    = &sms_native_session_create;
        load_fn_      = &sms_native_session_load;
        invoke_fn_    = &sms_native_session_invoke;
        dispose_fn_   = &sms_native_session_dispose;
        set_ui_cb_fn_ = &sms_native_set_ui_callbacks;
        set_ui_string_cb_fn_ = &sms_native_set_ui_string_callbacks;

        char err[512] = {};
        set_ui_cb_fn_(&sms_ui_get, &sms_ui_set, &sms_ui_invoke, err, static_cast<int>(sizeof(err)));
        if (err[0] != '\0') {
            UtilityFunctions::push_warning(String((
                "[ForgeRunner.Native] SMS set_ui_callbacks warning: " + std::string(err)).c_str()));
        }
        if (set_ui_string_cb_fn_ != nullptr) {
            char string_err[512] = {};
            set_ui_string_cb_fn_(&sms_ui_get_string, &sms_ui_set_string,
                                 string_err, static_cast<int>(sizeof(string_err)));
            if (string_err[0] != '\0') {
                UtilityFunctions::push_warning(String((
                    "[ForgeRunner.Native] SMS set_ui_string_callbacks warning: "
                    + std::string(string_err)).c_str()));
            }
        }

        loaded_ = true;
        UtilityFunctions::print("[ForgeRunner.Native] SMS runtime linked into forge_runner_native.");
        return true;
    }
#endif
    if (!lib_handle_) {
        UtilityFunctions::push_warning(String((
            "[ForgeRunner.Native] SMS library not found at " + lib_path.string() +
            " - SMS execution disabled.").c_str()));
        return false;
    }

    create_fn_    = reinterpret_cast<CreateFn> (platform_load_sym(lib_handle_, "sms_native_session_create"));
    load_fn_      = reinterpret_cast<LoadFn>   (platform_load_sym(lib_handle_, "sms_native_session_load"));
    invoke_fn_    = reinterpret_cast<InvokeFn> (platform_load_sym(lib_handle_, "sms_native_session_invoke"));
    aot_invoke_fn_       = reinterpret_cast<AotInvokeFn>    (platform_load_sym(lib_handle_, "sms_native_aot_invoke"));
    aot_lib_open_fn_     = reinterpret_cast<AotLibOpenFn>   (platform_load_sym(lib_handle_, "sms_native_aot_lib_open"));
    aot_lib_dispatch_fn_ = reinterpret_cast<AotLibDispatchFn>(platform_load_sym(lib_handle_, "sms_native_aot_lib_dispatch"));
    aot_lib_close_fn_    = reinterpret_cast<AotLibCloseFn>  (platform_load_sym(lib_handle_, "sms_native_aot_lib_close"));
    dispose_fn_   = reinterpret_cast<DisposeFn>(platform_load_sym(lib_handle_, "sms_native_session_dispose"));
    set_ui_cb_fn_ = reinterpret_cast<SetUiCbFn>(platform_load_sym(lib_handle_, "sms_native_set_ui_callbacks"));
    set_ui_string_cb_fn_ = reinterpret_cast<SetUiStringCbFn>(platform_load_sym(lib_handle_, "sms_native_set_ui_string_callbacks"));

    if (!create_fn_ || !load_fn_ || !invoke_fn_ || !dispose_fn_ || !set_ui_cb_fn_) {
        UtilityFunctions::push_warning("[ForgeRunner.Native] SMS library is missing required symbols.");
        platform_free_lib(lib_handle_);
        lib_handle_ = nullptr;
        return false;
    }

    char err[512] = {};
    set_ui_cb_fn_(&sms_ui_get, &sms_ui_set, &sms_ui_invoke, err, static_cast<int>(sizeof(err)));
    if (err[0] != '\0')
        UtilityFunctions::push_warning(String((
            "[ForgeRunner.Native] SMS set_ui_callbacks warning: " + std::string(err)).c_str()));
    if (set_ui_string_cb_fn_ != nullptr) {
        char string_err[512] = {};
        set_ui_string_cb_fn_(&sms_ui_get_string, &sms_ui_set_string,
                             string_err, static_cast<int>(sizeof(string_err)));
        if (string_err[0] != '\0') {
            UtilityFunctions::push_warning(String((
                "[ForgeRunner.Native] SMS set_ui_string_callbacks warning: "
                + std::string(string_err)).c_str()));
        }
    }

    loaded_ = true;
    UtilityFunctions::print("[ForgeRunner.Native] SMS native bridge loaded.");
    return true;
}

void SmsBridge::unload() {
    if (!lib_handle_) return;
    platform_free_lib(lib_handle_);
    lib_handle_   = nullptr;
    create_fn_    = nullptr;
    load_fn_      = nullptr;
    invoke_fn_    = nullptr;
    aot_invoke_fn_       = nullptr;
    aot_lib_open_fn_     = nullptr;
    aot_lib_dispatch_fn_ = nullptr;
    aot_lib_close_fn_    = nullptr;
    dispose_fn_   = nullptr;
    set_ui_cb_fn_ = nullptr;
    set_ui_string_cb_fn_ = nullptr;
    session_meta_.clear();
    loaded_       = false;
    set_sms_runtime_mode(SmsRuntimeMode::Interpreter);
}

std::int64_t SmsBridge::start_session(const std::string& script_path) {
    if (!loaded_) return -1;

    // sms_native_session_load expects script SOURCE, not a file path.
    std::ifstream f(script_path);
    if (!f.is_open()) {
        UtilityFunctions::push_warning(String((
            "[ForgeRunner.Native] SMS cannot open script: " + script_path).c_str()));
        return -1;
    }
    std::ostringstream ss;
    ss << f.rdbuf();
    const std::string source = ss.str();

    return start_session_from_source(source, script_path);
}

std::int64_t SmsBridge::start_session_from_source(const std::string& source, const std::string& source_label) {
    if (!loaded_) return -1;

    std::int64_t session = -1;
    char err[512] = {};
    if (create_fn_(&session, err, static_cast<int>(sizeof(err))) != 0 || session < 0) {
        UtilityFunctions::push_warning(String((
            "[ForgeRunner.Native] SMS session create failed: " + std::string(err)).c_str()));
        return -1;
    }
    if (load_fn_(session, source.c_str(), err, static_cast<int>(sizeof(err))) != 0) {
        last_error_ = err;
        std::string prefix = "[ForgeRunner.Native] SMS session load failed";
        if (!source_label.empty()) {
            prefix += " for '" + source_label + "'";
        }
        UtilityFunctions::push_warning(String(((prefix + ": " + std::string(err))).c_str()));
        dispose_fn_(session, nullptr, 0);
        return -1;
    }
    last_error_.clear();
    SessionMeta meta;
    meta.is_local_source = is_local_source_label(source_label);
    meta.source = source;

    // Try AOT lib compilation for all local sources (every OS, no interpreter).
    if (meta.is_local_source && aot_lib_open_fn_ != nullptr) {
        std::int64_t lib_id = -1;
        char aot_err[1024] = {};
        const int aot_rc = aot_lib_open_fn_(source.c_str(), &lib_id,
                                            aot_err, static_cast<int>(sizeof(aot_err)));
        if (aot_rc == 0) {
            meta.aot_lib_ready = true;
            meta.aot_lib_id    = lib_id;
            set_sms_runtime_mode(SmsRuntimeMode::Native);
            UtilityFunctions::print(String((
                "[ForgeRunner.Native] SMS AOT lib ready" +
                (source_label.empty() ? std::string() : " for '" + source_label + "'")).c_str()));
        } else {
            meta.aot_lib_failed = true;
            const std::string label_part = source_label.empty() ? std::string() : " for '" + source_label + "'";
            const std::string reason = std::string(aot_err);
            if (is_dev_mode()) {
                // DevMode: warn and fall back to interpreter so iteration stays fast.
                set_sms_runtime_mode(SmsRuntimeMode::Interpreter);
                UtilityFunctions::push_warning(String((
                    "[ForgeRunner.Native] SMS AOT compile failed" + label_part
                    + " (DevMode → interpreter fallback): " + reason).c_str()));
            } else {
                // Production: AOT is mandatory for local scripts - log as error.
                set_sms_runtime_mode(SmsRuntimeMode::Interpreter);
                UtilityFunctions::push_error(String((
                    "[ForgeRunner.Native] SMS AOT compile failed" + label_part
                    + " (no interpreter in production): " + reason).c_str()));
            }
        }
    } else {
        set_sms_runtime_mode(SmsRuntimeMode::Interpreter);
    }

    session_meta_[session] = std::move(meta);
    return session;
}

void SmsBridge::dispatch_event(std::int64_t session,
                               const std::string& object_id,
                               const std::string& event_name,
                               const std::string& payload_json) {
    if (!loaded_ || session < 0) return;
    struct DispatchDepthGuard {
        int& depth_ref;
        bool entered = false;
        explicit DispatchDepthGuard(int& depth) : depth_ref(depth) {
            depth_ref++;
            entered = true;
        }
        ~DispatchDepthGuard() {
            if (entered && depth_ref > 0) {
                depth_ref--;
            }
        }
    };
    static thread_local int dispatch_depth = 0;
    if (dispatch_depth >= kMaxSmsDispatchDepth) {
        const std::string msg = "RuntimeError: SMS dispatch recursion limit exceeded for '"
            + object_id + "." + event_name + "' (possible stack overflow).";
        UtilityFunctions::push_warning(String(("[ForgeRunner.Native] SMS dispatch failed for '" + object_id + "." + event_name + "': " + msg).c_str()));
        (void)try_quit_on_fatal_sms_error(msg);
        return;
    }
    DispatchDepthGuard depth_guard(dispatch_depth);

    const char* args_json = payload_json.empty() ? "[]" : payload_json.c_str();
    auto meta_it = session_meta_.find(session);

    // AOT lib path: compiled handler functions, called directly - no interpreter.
    if (meta_it != session_meta_.end() && meta_it->second.aot_lib_ready) {
        set_sms_runtime_mode(SmsRuntimeMode::Native);
        char aot_err[512] = {};
        const int aot_rc = aot_lib_dispatch_fn_(
            meta_it->second.aot_lib_id,
            object_id.c_str(), event_name.c_str(),
            aot_err, static_cast<int>(sizeof(aot_err)));
        if (aot_rc != 0) {
            const std::string msg = aot_err[0] != '\0' ? std::string(aot_err) : "unknown aot lib dispatch error";
            UtilityFunctions::push_warning(String((
                "[ForgeRunner.Native] SMS AOT dispatch error for '"
                + object_id + "." + event_name + "': " + msg).c_str()));
        }
        return;
    }

    // Interpreter fallback: HTTP scripts, or DevMode when AOT compile failed.
    const bool can_use_interpreter = meta_it == session_meta_.end()
        || !meta_it->second.is_local_source
        || (meta_it->second.aot_lib_failed && is_dev_mode());
    if (!can_use_interpreter) {
        return; // production local script with AOT failure - already logged as error
    }
    set_sms_runtime_mode(SmsRuntimeMode::Interpreter);
    std::int64_t result_session = -1;
    char err[512] = {};
    const int rc = invoke_fn_(session, object_id.c_str(), event_name.c_str(), args_json,
                              &result_session, err, static_cast<int>(sizeof(err)));
    if (rc != 0) {
        const std::string msg = err[0] != '\0' ? std::string(err) : std::string("unknown invoke error");
        if (sms_error_is_missing_handler(msg)) {
            return;
        }
        UtilityFunctions::push_warning(String(("[ForgeRunner.Native] SMS dispatch failed for '" + object_id + "." + event_name + "': " + msg).c_str()));
        (void)try_quit_on_fatal_sms_error(msg);
    }
}

void SmsBridge::dispose_session(std::int64_t session) {
    if (!loaded_ || session < 0) return;
    auto it = session_meta_.find(session);
    if (it != session_meta_.end()) {
        if (it->second.aot_lib_ready && aot_lib_close_fn_ != nullptr) {
            aot_lib_close_fn_(it->second.aot_lib_id);
        }
        session_meta_.erase(it);
    }
    if (session_meta_.empty()) {
        set_sms_runtime_mode(SmsRuntimeMode::Interpreter);
    }
    dispose_fn_(session, nullptr, 0);
}

} // namespace forge
