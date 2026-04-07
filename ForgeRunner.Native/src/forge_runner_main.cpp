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

#include "forge_runner_main.h"
#include "forge_ui_builder.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/base_button.hpp>
#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/file_dialog.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/h_box_container.hpp>
#include <godot_cpp/classes/http_request.hpp>
#include <godot_cpp/classes/item_list.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/line_edit.hpp>
#include <godot_cpp/classes/margin_container.hpp>
#include <godot_cpp/classes/option_button.hpp>
#include <godot_cpp/classes/progress_bar.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/popup_menu.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/slider.hpp>
#include <godot_cpp/classes/spin_box.hpp>
#include <godot_cpp/classes/text_edit.hpp>
#include <godot_cpp/classes/timer.hpp>
#include <godot_cpp/classes/tree.hpp>
#include <godot_cpp/classes/v_box_container.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace fs = std::filesystem;
using namespace godot;

namespace {
ForgeRunnerNativeMain* g_sms_main_instance = nullptr;

void sms_open_dialog_hook(const std::string& callback_name, const std::string& filter, bool save_mode) {
    if (g_sms_main_instance) {
        g_sms_main_instance->open_sms_file_dialog(callback_name, filter, save_mode);
    }
}
} // namespace

// ---------------------------------------------------------------------------
// GDClass boilerplate
// ---------------------------------------------------------------------------

void ForgeRunnerNativeMain::_bind_methods() {
    ClassDB::bind_method(D_METHOD("on_splash_timeout"),
                         &ForgeRunnerNativeMain::on_splash_timeout);
    ClassDB::bind_method(D_METHOD("on_sms_event", "object_id", "event_name"),
                         &ForgeRunnerNativeMain::on_sms_event);
    ClassDB::bind_method(D_METHOD("on_sms_bool_event", "value", "object_id", "event_name"),
                         &ForgeRunnerNativeMain::on_sms_bool_event);
    ClassDB::bind_method(D_METHOD("on_sms_text_event", "text", "object_id", "event_name"),
                         &ForgeRunnerNativeMain::on_sms_text_event);
    ClassDB::bind_method(D_METHOD("on_sms_value_event", "value", "object_id", "event_name"),
                         &ForgeRunnerNativeMain::on_sms_value_event);
    ClassDB::bind_method(D_METHOD("on_sms_item_event", "index", "object_id", "event_name"),
                         &ForgeRunnerNativeMain::on_sms_item_event);
    ClassDB::bind_method(D_METHOD("on_sms_popup_item_selected", "id", "object_id", "event_name"),
                         &ForgeRunnerNativeMain::on_sms_popup_item_selected);
    ClassDB::bind_method(D_METHOD("on_sms_popup_item_selected_by_index", "index", "object_id", "event_name"),
                         &ForgeRunnerNativeMain::on_sms_popup_item_selected_by_index);
    ClassDB::bind_method(D_METHOD("on_sms_file_dialog_selected", "path", "callback_name", "dialog_id"),
                         &ForgeRunnerNativeMain::on_sms_file_dialog_selected);
    ClassDB::bind_method(D_METHOD("on_sms_file_dialog_canceled", "callback_name", "dialog_id"),
                         &ForgeRunnerNativeMain::on_sms_file_dialog_canceled);
    ClassDB::bind_method(D_METHOD("on_sms_tree_button_clicked", "item", "column", "id", "mouse_button_index", "object_id", "event_name"),
                         &ForgeRunnerNativeMain::on_sms_tree_button_clicked);
}

// ---------------------------------------------------------------------------
// _ready
// ---------------------------------------------------------------------------

void ForgeRunnerNativeMain::_ready() {
    UtilityFunctions::print("[ForgeRunner.Native] _ready");
    set_process(true);
    g_sms_main_instance = this;
    forge::set_ui_open_dialog_hook(&sms_open_dialog_hook);

    if (Viewport* vp = get_viewport()) {
        vp->connect("size_changed", callable_mp(this, &ForgeRunnerNativeMain::on_viewport_size_changed));
    }

    const char* env_url = std::getenv("FORGE_RUNNER_URL");
    if (!env_url || env_url[0] == '\0') {
        UtilityFunctions::push_warning("[ForgeRunner.Native] FORGE_RUNNER_URL is not set. Falling back to res:/app.sml.");
        show_sml("res:/app.sml");
        return;
    }

    const std::string url(env_url);

    if (is_http_url(url)) {
        show_loading_screen();
        start_manifest_download(url);
        return;
    }

    const std::string path = file_url_to_path(url);
    if (path.empty()) {
        show_error("Unsupported URL scheme (only file:// is supported locally): " + url);
        return;
    }

    show_sml(path);
}

void ForgeRunnerNativeMain::_process(double) {
    if (sms_session_ < 0) {
        return;
    }
    std::string event_name;
    std::string payload_json;
    while (forge::pop_benchmark_event(event_name, payload_json)) {
        sms_bridge_.dispatch_event(sms_session_, "benchmark", event_name, payload_json);
    }
}

// ---------------------------------------------------------------------------
// URL resolution
// ---------------------------------------------------------------------------

std::string ForgeRunnerNativeMain::file_url_to_path(const std::string& url) {
    if (url.size() < 7 || url.substr(0, 7) != "file://") return {};

    std::string path = url.substr(7); // strip "file://"

    // file://localhost/... → /...
    if (path.size() >= 9 && path.substr(0, 9) == "localhost") path = path.substr(9);

    // Ensure leading slash
    if (!path.empty() && path[0] != '/') path = "/" + path;

    return path;
}

// ---------------------------------------------------------------------------
// File loading
// ---------------------------------------------------------------------------

std::string ForgeRunnerNativeMain::load_file(const std::string& path) {
    if (path.rfind("res:/", 0) == 0 || path.rfind("res://", 0) == 0 ||
        path.rfind("user:/", 0) == 0 || path.rfind("user://", 0) == 0) {
        std::string godot_path = path;
        if (godot_path.rfind("res:/", 0) == 0 && godot_path.rfind("res://", 0) != 0) {
            godot_path = "res://" + godot_path.substr(5);
        } else if (godot_path.rfind("user:/", 0) == 0 && godot_path.rfind("user://", 0) != 0) {
            godot_path = "user://" + godot_path.substr(6);
        }
        Ref<FileAccess> f = FileAccess::open(String(godot_path.c_str()), FileAccess::ModeFlags::READ);
        if (f.is_null()) return {};
        const String text = f->get_as_text();
        return text.utf8().get_data();
    }
    std::ifstream f(path);
    if (!f.is_open()) return {};
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

// ---------------------------------------------------------------------------
// SML loading + UI build
// ---------------------------------------------------------------------------

void ForgeRunnerNativeMain::show_sml(const std::string& path) {
    clear_content();

    const auto src = load_file(path);
    if (src.empty()) {
        show_error("Could not read file: " + path);
        return;
    }

    smlcore::Document doc;
    try {
        doc = smlcore::parse_document(src);
    } catch (const std::exception& ex) {
        show_error(std::string("SML parse error: ") + ex.what());
        return;
    }

    if (doc.roots.empty()) {
        show_error("SML document has no root node: " + path);
        return;
    }

    // Determine base directory and appres root
    const auto last_sep = path.rfind('/');
    const std::string base_dir = (last_sep != std::string::npos)
                                 ? path.substr(0, last_sep) : ".";

    const char* env_appres = std::getenv("FORGE_RUNNER_APPRES_ROOT");
    std::string appres_root;
    if (env_appres && env_appres[0] != '\0') {
        appres_root = env_appres;
    } else {
        // Default: Godot project root (ForgeRunner.Native/) so that appRes:/
        // paths resolve relative to the native app, not the SML file location.
        String proj_root = ProjectSettings::get_singleton()->globalize_path("res://");
        appres_root = std::string(proj_root.utf8().get_data());
        if (!appres_root.empty() && appres_root.back() == '/')
            appres_root.pop_back();
    }

    // Build UI
    forge::UiBuilder builder(base_dir, appres_root);
    forge::WindowConfig win_cfg;
    Control* ui = builder.build(doc, win_cfg);

    // Apply window settings
    Window* win = get_window();
    if (win) {
        if (win_cfg.is_splash) {
            // Splash: project.godot sets extend_to_title, always_on_top,
            // transparent, resizable=false. Only hide minimize+maximize so
            // just the red close button remains visible.
            win->set_flag(Window::FLAG_MINIMIZE_DISABLED, true);
            win->set_flag(Window::FLAG_MAXIMIZE_DISABLED, true);
            if (win_cfg.width > 0 && win_cfg.height > 0)
                win->set_size(Vector2i(win_cfg.width, win_cfg.height));
        } else {
            // Window: reset splash-mode flags, then apply SML-specified values.
            win->set_flag(Window::FLAG_MINIMIZE_DISABLED, false);
            win->set_flag(Window::FLAG_MAXIMIZE_DISABLED, false);
            win->set_flag(Window::FLAG_ALWAYS_ON_TOP,   false);
            win->set_flag(Window::FLAG_RESIZE_DISABLED, false);
            win->set_flag(Window::FLAG_EXTEND_TO_TITLE, win_cfg.extend_to_title);
            win->set_flag(Window::FLAG_BORDERLESS,      win_cfg.borderless);
            if (!win_cfg.title.empty())
                win->set_title(String(win_cfg.title.c_str()));
            if (win_cfg.width > 0 && win_cfg.height > 0)
                win->set_size(Vector2i(win_cfg.width, win_cfg.height));
            if (win_cfg.min_width > 0 && win_cfg.min_height > 0)
                win->set_min_size(Vector2i(win_cfg.min_width, win_cfg.min_height));
            if (win_cfg.center_on_screen)
                win->move_to_center();
        }
    }

    add_child(ui);
    content_root_ = ui;

    // Start SMS if a companion .sms script exists
    if (!win_cfg.is_splash) {
        const std::string root_id = doc.roots[0].get_value("id", "");
        start_sms(path, doc.roots[0].name, root_id);
    }

    // Splash: schedule next load
    if (win_cfg.is_splash && !win_cfg.splash_load_on_ready.empty()) {
        splash_next_path_ = base_dir;
        if (!splash_next_path_.empty() && splash_next_path_.back() != '/') {
            splash_next_path_ += "/";
        }
        splash_next_path_ += win_cfg.splash_load_on_ready;

        if (splash_timer_) {
            splash_timer_->stop();
            splash_timer_->queue_free();
        }
        splash_timer_ = memnew(Timer);
        splash_timer_->set_one_shot(true);
        splash_timer_->set_wait_time(
            static_cast<double>(std::max(1, win_cfg.splash_duration_ms)) / 1000.0);
        splash_timer_->connect("timeout",
            Callable(this, "on_splash_timeout"));
        add_child(splash_timer_);
        splash_timer_->start();
    }

    UtilityFunctions::print(String("[ForgeRunner.Native] loaded: ") +
                            String(path.c_str()));
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

void ForgeRunnerNativeMain::clear_content() {
    stop_sms();
    if (http_request_) {
        http_request_->cancel_request();
        http_request_->queue_free();
        http_request_ = nullptr;
    }
    splash_progress_ = nullptr;
    download_queue_.clear();
    cached_asset_hashes_.clear();
    download_queue_idx_ = 0;
    download_retry_     = 0;
    if (content_root_) {
        content_root_->queue_free();
        content_root_ = nullptr;
    }
    if (splash_timer_) {
        splash_timer_->stop();
        splash_timer_->queue_free();
        splash_timer_ = nullptr;
    }
    splash_next_path_.clear();
}

void ForgeRunnerNativeMain::show_error(const std::string& msg) {
    UtilityFunctions::push_error(String(("[ForgeRunner.Native] " + msg).c_str()));

    auto* lbl = memnew(Label);
    lbl->set_text(String(msg.c_str()));
    lbl->set_anchor_and_offset(SIDE_LEFT,   0.0f, 16.0f);
    lbl->set_anchor_and_offset(SIDE_TOP,    0.0f, 16.0f);
    lbl->set_anchor_and_offset(SIDE_RIGHT,  1.0f, -16.0f);
    lbl->set_anchor_and_offset(SIDE_BOTTOM, 1.0f, -16.0f);
    lbl->set_autowrap_mode(TextServer::AUTOWRAP_WORD_SMART);
    add_child(lbl);
    content_root_ = lbl;
}

void ForgeRunnerNativeMain::on_splash_timeout() {
    if (splash_next_path_.empty()) return;
    const auto next = splash_next_path_;
    splash_next_path_.clear();
    show_sml(next);
}

// ---------------------------------------------------------------------------
// SMS integration
// ---------------------------------------------------------------------------

void ForgeRunnerNativeMain::start_sms(const std::string& sml_path, const std::string& root_name, const std::string& root_id) {
    std::string script_path = sml_path;
    const std::size_t last_slash = script_path.find_last_of("/\\");
    const std::size_t last_dot = script_path.find_last_of('.');
    if (last_dot != std::string::npos && (last_slash == std::string::npos || last_dot > last_slash)) {
        script_path = script_path.substr(0, last_dot) + ".sms";
    } else {
        script_path += ".sms";
    }

    const std::string script_source = load_file(script_path);
    if (script_source.empty()) return;

    // Resolve repo root via FORGE_RUNNER_APPRES_ROOT (set by run.sh to
    // <repo>/ForgeRunner.Native), so parent_path() gives us <repo>.
    // Fall back to two levels above res:// if the env var is absent.
    std::string repo_root;
    const char* env_appres = std::getenv("FORGE_RUNNER_APPRES_ROOT");
    if (env_appres && env_appres[0] != '\0') {
        repo_root = fs::path(env_appres).parent_path().string();
    } else {
        String proj_root = ProjectSettings::get_singleton()->globalize_path("res://");
        repo_root = fs::path(proj_root.utf8().get_data())
                        .lexically_normal().parent_path().parent_path().string();
    }

    if (!sms_bridge_.load(repo_root)) return;

    sms_session_ = sms_bridge_.start_session_from_source(script_source, script_path);
    if (sms_session_ < 0) return;

    popup_item_id_map_.clear();
    popup_item_index_to_id_map_.clear();
    bind_popup_menu_events(content_root_);

    // Connect signals for all id-registered controls
    const auto& id_map = forge::SmsBridge::id_map();
    for (const auto& [id, ctrl] : id_map) {
        if (!ctrl) continue;
        const String gid(id.c_str());

        // 0-arg signals
        auto cb0 = [&](Control* c, const char* signal, const char* event) {
            c->connect(signal,
                callable_mp(this, &ForgeRunnerNativeMain::on_sms_event)
                    .bind(gid, String(event)));
        };
        // bool signals
        auto cbBool = [&](Control* c, const char* signal, const char* event) {
            c->connect(signal,
                callable_mp(this, &ForgeRunnerNativeMain::on_sms_bool_event)
                    .bind(gid, String(event)));
        };
        // text signals
        auto cbText = [&](Control* c, const char* signal, const char* event) {
            c->connect(signal,
                callable_mp(this, &ForgeRunnerNativeMain::on_sms_text_event)
                    .bind(gid, String(event)));
        };
        // numeric value signals
        auto cbVal = [&](Control* c, const char* signal, const char* event) {
            c->connect(signal,
                callable_mp(this, &ForgeRunnerNativeMain::on_sms_value_event)
                    .bind(gid, String(event)));
        };
        // int item signals
        auto cbItem = [&](Control* c, const char* signal, const char* event) {
            c->connect(signal,
                callable_mp(this, &ForgeRunnerNativeMain::on_sms_item_event)
                    .bind(gid, String(event)));
        };
        auto cbItemText = [&](Control* c, const char* signal, const char* event) {
            c->connect(signal,
                callable_mp(this, &ForgeRunnerNativeMain::on_sms_item_text_event)
                    .bind(gid, String(event)));
        };

        if (auto* btn = Object::cast_to<BaseButton>(ctrl)) {
            cb0(btn,   "pressed",  "clicked");
            cbBool(btn, "toggled", "toggled");
        }
        if (auto* le = Object::cast_to<LineEdit>(ctrl)) {
            cbText(le, "text_changed",    "textChanged");
            cb0(le,    "text_submitted",  "textSubmitted");
        }
        if (auto* te = Object::cast_to<TextEdit>(ctrl)) {
            cb0(te, "text_changed", "textChanged");
        }
        if (auto* sb = Object::cast_to<SpinBox>(ctrl)) {
            cbVal(sb, "value_changed", "valueChanged");
        }
        if (auto* sl = Object::cast_to<Slider>(ctrl)) {
            cbVal(sl, "value_changed", "valueChanged");
        }
        if (auto* ob = Object::cast_to<OptionButton>(ctrl)) {
            cbItem(ob, "item_selected", "itemSelected");
        }
        if (auto* il = Object::cast_to<ItemList>(ctrl)) {
            cbItem(il, "item_selected", "itemSelected");
        }
        if (auto* tree = Object::cast_to<Tree>(ctrl)) {
            cb0(tree, "item_selected", "itemSelected");
            tree->connect("button_clicked",
                callable_mp(this, &ForgeRunnerNativeMain::on_sms_tree_button_clicked)
                    .bind(gid, String("treeItemToggled")));
        }
        if (ctrl->has_signal("boneSelected"))      cbText(ctrl, "boneSelected", "boneSelected");
        if (ctrl->has_signal("poseChanged"))       cbText(ctrl, "poseChanged", "poseChanged");
        if (ctrl->has_signal("poseReset"))         cb0(ctrl, "poseReset", "poseReset");
        if (ctrl->has_signal("scenePropAdded"))    cbItemText(ctrl, "scenePropAdded", "scenePropAdded");
        if (ctrl->has_signal("scenePropRemoved"))  cbItem(ctrl, "scenePropRemoved", "scenePropRemoved");
        if (ctrl->has_signal("objectSelected"))    cbItem(ctrl, "objectSelected", "objectSelected");
        if (ctrl->has_signal("objectMoved"))       cbItemText(ctrl, "objectMoved", "objectMoved");
        if (ctrl->has_signal("keyframeAdded"))     cbItemText(ctrl, "keyframeAdded", "keyframeAdded");
        if (ctrl->has_signal("keyframeRemoved"))   cbItem(ctrl, "keyframeRemoved", "keyframeRemoved");
        if (ctrl->has_signal("frameChanged"))      cbItem(ctrl, "frameChanged", "frameChanged");
        if (ctrl->has_signal("playbackStarted"))   cb0(ctrl, "playbackStarted", "playbackStarted");
        if (ctrl->has_signal("playbackStopped"))   cb0(ctrl, "playbackStopped", "playbackStopped");

        // SpeechRecognizer custom control signals.
        if (ctrl->has_signal("started"))           cb0(ctrl, "started", "started");
        if (ctrl->has_signal("stopped"))           cb0(ctrl, "stopped", "stopped");
        if (ctrl->has_signal("rawResult"))         cbText(ctrl, "rawResult", "rawResult");
        if (ctrl->has_signal("partialResult"))     cbText(ctrl, "partialResult", "partialResult");
        if (ctrl->has_signal("result"))            cbText(ctrl, "result", "result");
        if (ctrl->has_signal("error"))             cbText(ctrl, "error", "error");
        if (ctrl->has_signal("started") || ctrl->has_signal("result") || ctrl->has_signal("error")) {
            UtilityFunctions::print(
                String(("[ForgeRunner.Native][SMSDBG] bound speech signals for id=" + id).c_str()));
        }
    }

    // Dispatch ready event
    const std::string root_lower = [&] {
        std::string s = root_name;
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::tolower(c); });
        return s;
    }();
    sms_bridge_.dispatch_event(sms_session_, root_lower, "ready");
    if (!root_id.empty() && root_id != root_lower) {
        sms_bridge_.dispatch_event(sms_session_, root_id, "ready");
    }
    {
        static constexpr std::string_view kAhimsaComments[] = {
            "// Do not harm any living being.",
            "// Tu keinem Lebewesen Leid an.",
            "// Ne dama\xc4\x9du iun ajn vivantan esta\xc4\xb5on.",
        };
        bool has_ahimsa = false;
        for (const auto& c : kAhimsaComments) {
            if (std::string_view(script_source).substr(0, c.size()) == c) { has_ahimsa = true; break; }
        }
        if (has_ahimsa) {
            UtilityFunctions::print("[ForgeRunner.Native] \xf0\x9f\x8c\xb1 Ahimsa \xe2\x80\x94 Do not harm any living being.");
        }
    }
    UtilityFunctions::print(String(("[ForgeRunner.Native] SMS session started: " + script_path).c_str()));
}

void ForgeRunnerNativeMain::stop_sms() {
    forge::reset_benchmark_runtime();
    if (sms_session_ >= 0) {
        sms_bridge_.dispose_session(sms_session_);
        sms_session_ = -1;
    }
    for (auto& [_, dlg] : open_file_dialogs_) {
        if (dlg) {
            dlg->queue_free();
        }
    }
    open_file_dialogs_.clear();
    popup_item_id_map_.clear();
    popup_item_index_to_id_map_.clear();
    forge::SmsBridge::id_map().clear();
}

void ForgeRunnerNativeMain::open_sms_file_dialog(const std::string& callback_name, const std::string& filter, bool save_mode) {
    if (callback_name.empty()) {
        return;
    }
    auto* tree = get_tree();
    if (!tree || !tree->get_root()) {
        UtilityFunctions::push_warning("[ForgeRunner.Native] ui.open*FileDialog: scene tree not available.");
        return;
    }

    auto* dialog = memnew(FileDialog);
    dialog->set_access(FileDialog::ACCESS_FILESYSTEM);
    dialog->set_file_mode(save_mode ? FileDialog::FILE_MODE_SAVE_FILE : FileDialog::FILE_MODE_OPEN_FILE);
    dialog->set_title(save_mode ? "Save File" : "Open File");
    dialog->set_size(Vector2i(800, 600));

    PackedStringArray filters;
    if (!filter.empty()) {
        std::stringstream ss(filter);
        std::string token;
        while (std::getline(ss, token, ',')) {
            const auto start = token.find_first_not_of(" \t\r\n");
            if (start == std::string::npos) {
                token.clear();
            } else {
                token.erase(0, start);
                const auto end = token.find_last_not_of(" \t\r\n");
                token.erase(end + 1);
            }
            if (!token.empty()) {
                filters.push_back(String(token.c_str()));
            }
        }
    }
    if (filters.size() == 0) {
        filters.push_back(save_mode ? String("*.scene") : String("*.*"));
    }
    dialog->set_filters(filters);

    tree->get_root()->add_child(dialog);
    const int64_t dialog_id = static_cast<int64_t>(dialog->get_instance_id());
    open_file_dialogs_[dialog_id] = dialog;
    const String cb(callback_name.c_str());
    dialog->connect("file_selected",
        callable_mp(this, &ForgeRunnerNativeMain::on_sms_file_dialog_selected).bind(cb, dialog_id));
    dialog->connect("canceled",
        callable_mp(this, &ForgeRunnerNativeMain::on_sms_file_dialog_canceled).bind(cb, dialog_id));
    dialog->call_deferred("popup_centered");
}

void ForgeRunnerNativeMain::bind_popup_menu_events(Node* node) {
    if (!node) return;

    if (auto* popup = Object::cast_to<PopupMenu>(node)) {
        String popup_id;
        if (popup->has_meta("sml_id")) {
            const Variant popup_meta = popup->get_meta("sml_id");
            if (popup_meta.get_type() == Variant::STRING) {
                popup_id = popup_meta;
            }
        }
        if (popup_id.is_empty()) {
            popup_id = popup->get_name();
        }

        const std::string popup_key = popup_id.utf8().get_data();
        if (!popup_key.empty()) {
            auto& map = popup_item_id_map_[popup_key];
            auto& index_map = popup_item_index_to_id_map_[popup_key];
            map.clear();
            index_map.clear();
            const int count = popup->get_item_count();
            for (int i = 0; i < count; ++i) {
                const int item_id = popup->get_item_id(i);
                index_map[i] = item_id;
                String item_sml_id;
                const Variant md = popup->get_item_metadata(i);
                if (md.get_type() == Variant::STRING) {
                    item_sml_id = md;
                }
                if (item_sml_id.is_empty()) {
                    item_sml_id = popup->get_item_text(i);
                }
                if (!item_sml_id.is_empty()) {
                    map[item_id] = item_sml_id.utf8().get_data();
                }
            }

            popup->connect(
                "id_pressed",
                callable_mp(this, &ForgeRunnerNativeMain::on_sms_popup_item_selected)
                    .bind(popup_id, String("menuItemSelected")));
        }
    }

    const int child_count = node->get_child_count();
    for (int i = 0; i < child_count; ++i) {
        if (auto* child = Object::cast_to<Node>(node->get_child(i))) {
            bind_popup_menu_events(child);
        }
    }
}

// ---------------------------------------------------------------------------
// SMS event handlers
// ---------------------------------------------------------------------------

void ForgeRunnerNativeMain::on_sms_event(String object_id, String event_name) {
    UtilityFunctions::print(String(("[ForgeRunner.Native][SMSDBG] event " +
        std::string(object_id.utf8().get_data()) + "." +
        std::string(event_name.utf8().get_data())).c_str()));
    sms_bridge_.dispatch_event(sms_session_,
        object_id.utf8().get_data(), event_name.utf8().get_data());
}

void ForgeRunnerNativeMain::on_sms_bool_event(bool value, String object_id, String event_name) {
    sms_bridge_.dispatch_event(sms_session_,
        object_id.utf8().get_data(), event_name.utf8().get_data(),
        value ? "[true]" : "[false]");
}

void ForgeRunnerNativeMain::on_viewport_size_changed() {
    if (sms_session_ < 0) return;
    Viewport* vp = get_viewport();
    if (!vp) return;
    const Vector2 sz = vp->get_visible_rect().size;
    const bool is_landscape = sz.x > sz.y;
    const std::string orientation = is_landscape ? "landscape" : "portrait";
    const std::string payload = "[\"" + orientation + "\"]";
    sms_bridge_.dispatch_event(sms_session_, "mainWindow", "orientationChanged", payload);
}

void ForgeRunnerNativeMain::on_sms_text_event(String text, String object_id, String event_name) {
    UtilityFunctions::print(String(("[ForgeRunner.Native][SMSDBG] text-event " +
        std::string(object_id.utf8().get_data()) + "." +
        std::string(event_name.utf8().get_data()) + " -> " +
        std::string(text.utf8().get_data())).c_str()));
    std::string json = "[\"";
    const std::string raw = text.utf8().get_data();
    for (unsigned char c : raw) {
        if      (c == '"')  json += "\\\"";
        else if (c == '\\') json += "\\\\";
        else if (c == '\n') json += "\\n";
        else                json += static_cast<char>(c);
    }
    json += "\"]";
    sms_bridge_.dispatch_event(sms_session_,
        object_id.utf8().get_data(), event_name.utf8().get_data(), json);
}

void ForgeRunnerNativeMain::on_sms_value_event(double value, String object_id, String event_name) {
    char buf[64];
    std::snprintf(buf, sizeof(buf), "[%g]", value);
    sms_bridge_.dispatch_event(sms_session_,
        object_id.utf8().get_data(), event_name.utf8().get_data(), buf);
}

void ForgeRunnerNativeMain::on_sms_item_event(int index, String object_id, String event_name) {
    char buf[32];
    std::snprintf(buf, sizeof(buf), "[%d]", index);
    sms_bridge_.dispatch_event(sms_session_,
        object_id.utf8().get_data(), event_name.utf8().get_data(), buf);
}

void ForgeRunnerNativeMain::on_sms_item_text_event(int index, String text, String object_id, String event_name) {
    std::string escaped;
    const std::string raw = text.utf8().get_data();
    escaped.reserve(raw.size() + 8);
    for (unsigned char c : raw) {
        if      (c == '"')  escaped += "\\\"";
        else if (c == '\\') escaped += "\\\\";
        else if (c == '\n') escaped += "\\n";
        else                escaped += static_cast<char>(c);
    }
    const std::string payload = "[" + std::to_string(index) + ",\"" + escaped + "\"]";
    sms_bridge_.dispatch_event(sms_session_,
        object_id.utf8().get_data(), event_name.utf8().get_data(), payload);
}

void ForgeRunnerNativeMain::on_sms_popup_item_selected(int id, String object_id, String event_name) {
    String clicked = String::num_int64(id);
    const auto popup_it = popup_item_id_map_.find(object_id.utf8().get_data());
    if (popup_it != popup_item_id_map_.end()) {
        const auto item_it = popup_it->second.find(id);
        if (item_it != popup_it->second.end() && !item_it->second.empty()) {
            clicked = String(item_it->second.c_str());
        }
    }
    std::string escaped;
    const std::string raw = clicked.utf8().get_data();
    escaped.reserve(raw.size() + 8);
    for (unsigned char c : raw) {
        if      (c == '"')  escaped += "\\\"";
        else if (c == '\\') escaped += "\\\\";
        else if (c == '\n') escaped += "\\n";
        else                escaped += static_cast<char>(c);
    }
    const std::string payload = "[\"" + escaped + "\"]";
    if (!clicked.is_empty()) {
        sms_bridge_.dispatch_event(sms_session_,
            clicked.utf8().get_data(), "clicked", "[]");
    }
    sms_bridge_.dispatch_event(sms_session_,
        object_id.utf8().get_data(), event_name.utf8().get_data(), payload);
}

void ForgeRunnerNativeMain::on_sms_popup_item_selected_by_index(int index, String object_id, String event_name) {
    const auto index_it = popup_item_index_to_id_map_.find(object_id.utf8().get_data());
    if (index_it == popup_item_index_to_id_map_.end()) {
        return;
    }
    const auto id_it = index_it->second.find(index);
    if (id_it != index_it->second.end()) {
        on_sms_popup_item_selected(id_it->second, object_id, event_name);
        return;
    }
    on_sms_popup_item_selected(index, object_id, event_name);
}

void ForgeRunnerNativeMain::on_sms_file_dialog_selected(String path, String callback_name, int64_t dialog_id) {
    if (dialog_id > 0) {
        const auto it = open_file_dialogs_.find(dialog_id);
        if (it != open_file_dialogs_.end()) {
            if (it->second) {
                it->second->queue_free();
            }
            open_file_dialogs_.erase(it);
        }
    }
    std::string escaped;
    const std::string raw = path.utf8().get_data();
    escaped.reserve(raw.size() + 8);
    for (unsigned char c : raw) {
        if      (c == '"')  escaped += "\\\"";
        else if (c == '\\') escaped += "\\\\";
        else if (c == '\n') escaped += "\\n";
        else                escaped += static_cast<char>(c);
    }
    const std::string payload = "[\"" + escaped + "\"]";
    sms_bridge_.dispatch_event(sms_session_, "ui", callback_name.utf8().get_data(), payload);
}

void ForgeRunnerNativeMain::on_sms_file_dialog_canceled(String, int64_t dialog_id) {
    if (dialog_id > 0) {
        const auto it = open_file_dialogs_.find(dialog_id);
        if (it != open_file_dialogs_.end()) {
            if (it->second) {
                it->second->queue_free();
            }
            open_file_dialogs_.erase(it);
        }
    }
}

void ForgeRunnerNativeMain::on_sms_tree_button_clicked(TreeItem*, int, int id, int, String object_id, String event_name) {
    const std::string payload = "[" + std::to_string(id) + ",false]";
    sms_bridge_.dispatch_event(sms_session_,
        object_id.utf8().get_data(), event_name.utf8().get_data(), payload);
}

// ---------------------------------------------------------------------------
// HTTP / Manifest download
// ---------------------------------------------------------------------------

bool ForgeRunnerNativeMain::is_http_url(const std::string& url) {
    return url.substr(0, 7) == "http://" || url.substr(0, 8) == "https://";
}

std::string ForgeRunnerNativeMain::per_manifest_cache_root(const std::string& manifest_url) {
    const auto url_hash = forge::AssetCache::sha256_of(
        std::vector<uint8_t>(manifest_url.begin(), manifest_url.end()));
    return forge::forge_cache_dir() + "/" + url_hash;
}

std::string ForgeRunnerNativeMain::resolve_asset_url(const std::string& manifest_url,
                                                      const std::string& base_url,
                                                      const std::string& raw_url) {
    if (raw_url.find("://") != std::string::npos) return raw_url;  // already absolute
    const auto& ref  = base_url.empty() ? manifest_url : base_url;
    const auto  last = ref.rfind('/');
    if (last == std::string::npos) return raw_url;
    return ref.substr(0, last + 1) + raw_url;
}

std::string ForgeRunnerNativeMain::normalize_asset_path(const std::string& path) {
    std::string out;
    out.reserve(path.size());
    for (char c : path) out += (c == '\\') ? '/' : c;
    // Strip leading slashes
    while (!out.empty() && out[0] == '/') out.erase(out.begin());
    // Reject path traversal
    std::istringstream ss(out);
    std::string seg;
    while (std::getline(ss, seg, '/')) {
        if (seg == "..") return {};
    }
    return out;
}

bool ForgeRunnerNativeMain::save_file_atomic(const std::string& path,
                                              const void* data, std::size_t size) {
    std::error_code ec;
    fs::create_directories(fs::path(path).parent_path(), ec);
    if (ec) return false;
    const std::string tmp = path + ".tmp";
    {
        std::ofstream f(tmp, std::ios::binary | std::ios::trunc);
        if (!f.is_open()) return false;
        if (size > 0)
            f.write(static_cast<const char*>(data), static_cast<std::streamsize>(size));
        if (!f.good()) return false;
    }
    if (fs::exists(path)) fs::remove(path, ec);
    fs::rename(tmp, path, ec);
    return !ec;
}

// ---------------------------------------------------------------------------
// Manifest metadata (index of downloaded assets for delta detection)
// Format: simple line-based key/value + Asset { } blocks
// ---------------------------------------------------------------------------

static std::string meta_extract_quoted(const std::string& line, const std::string& key) {
    const std::string prefix = key + ": \"";
    const auto pos = line.find(prefix);
    if (pos == std::string::npos) return {};
    const auto start = pos + prefix.size();
    size_t end = start;
    while (end < line.size()) {
        if (line[end] == '"' && (end == start || line[end - 1] != '\\')) break;
        ++end;
    }
    if (end >= line.size()) return {};
    // Unescape
    std::string out;
    for (size_t i = start; i < end; ++i) {
        if (line[i] == '\\' && i + 1 < end) { ++i; out += line[i]; }
        else out += line[i];
    }
    return out;
}

static std::string meta_escape(const std::string& s) {
    std::string out;
    for (unsigned char c : s) {
        if      (c == '"')  out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else                out += static_cast<char>(c);
    }
    return out;
}

bool ForgeRunnerNativeMain::load_manifest_meta(
    const std::string& cache_root,
    std::string& out_manifest_sha256,
    std::string& out_entry_point,
    std::unordered_map<std::string, std::string>& out_hashes)
{
    out_manifest_sha256.clear();
    out_entry_point = "app.sml";
    out_hashes.clear();

    std::ifstream f(cache_root + "/metadata.sml");
    if (!f.is_open()) return false;

    bool        in_asset = false;
    std::string cur_path, cur_sha256;
    std::string line;

    while (std::getline(f, line)) {
        size_t i = 0;
        while (i < line.size() && (line[i] == ' ' || line[i] == '\t')) ++i;
        const auto tl = line.substr(i);

        if (tl.find("Asset {") != std::string::npos) {
            in_asset = true;
            cur_path.clear(); cur_sha256.clear();
        } else if (in_asset && tl == "}") {
            if (!cur_path.empty() && !cur_sha256.empty())
                out_hashes[cur_path] = cur_sha256;
            in_asset = false;
        } else if (in_asset) {
            if      (tl.rfind("path:",   0) == 0) cur_path   = meta_extract_quoted(tl, "path");
            else if (tl.rfind("sha256:", 0) == 0) cur_sha256 = meta_extract_quoted(tl, "sha256");
        } else {
            if      (tl.rfind("manifestSha256:", 0) == 0)
                out_manifest_sha256 = meta_extract_quoted(tl, "manifestSha256");
            else if (tl.rfind("entryPoint:", 0) == 0)
                out_entry_point = meta_extract_quoted(tl, "entryPoint");
        }
    }
    return !out_manifest_sha256.empty();
}

void ForgeRunnerNativeMain::save_manifest_meta(
    const std::string& cache_root,
    const std::string& manifest_sha256,
    const std::string& entry_point,
    const std::vector<ManifestAsset>& assets)
{
    std::error_code ec;
    fs::create_directories(cache_root, ec);

    const std::string path = cache_root + "/metadata.sml";
    const std::string tmp  = path + ".tmp";
    {
        std::ofstream f(tmp);
        if (!f.is_open()) return;
        f << "ManifestMeta {\n";
        f << "    manifestSha256: \"" << meta_escape(manifest_sha256) << "\"\n";
        f << "    entryPoint: \""     << meta_escape(entry_point)     << "\"\n";
        for (const auto& a : assets) {
            f << "    Asset {\n";
            f << "        path: \""   << meta_escape(a.path)   << "\"\n";
            f << "        sha256: \"" << meta_escape(a.sha256) << "\"\n";
            f << "    }\n";
        }
        f << "}\n";
    }
    if (fs::exists(path)) fs::remove(path, ec);
    fs::rename(tmp, path, ec);
}

// ---------------------------------------------------------------------------
// Loading screen (shown while manifest/assets are being downloaded)
// ---------------------------------------------------------------------------

void ForgeRunnerNativeMain::show_loading_screen() {
    clear_content();

    auto* root_ctrl = memnew(Control);
    root_ctrl->set_anchor_and_offset(SIDE_LEFT,   0.0f, 0.0f);
    root_ctrl->set_anchor_and_offset(SIDE_TOP,    0.0f, 0.0f);
    root_ctrl->set_anchor_and_offset(SIDE_RIGHT,  1.0f, 0.0f);
    root_ctrl->set_anchor_and_offset(SIDE_BOTTOM, 1.0f, 0.0f);

    auto* margin = memnew(MarginContainer);
    margin->set_anchor_and_offset(SIDE_LEFT,   0.0f, 0.0f);
    margin->set_anchor_and_offset(SIDE_TOP,    0.0f, 0.0f);
    margin->set_anchor_and_offset(SIDE_RIGHT,  1.0f, 0.0f);
    margin->set_anchor_and_offset(SIDE_BOTTOM, 1.0f, 0.0f);
    margin->add_theme_constant_override("margin_left",   32);
    margin->add_theme_constant_override("margin_right",  32);
    margin->add_theme_constant_override("margin_top",    32);
    margin->add_theme_constant_override("margin_bottom", 32);
    root_ctrl->add_child(margin);

    auto* vbox = memnew(VBoxContainer);
    vbox->set_alignment(BoxContainer::ALIGNMENT_CENTER);
    vbox->set_anchors_preset(Control::PRESET_CENTER);
    vbox->set_custom_minimum_size(Vector2(400, 80));
    margin->add_child(vbox);

    auto* lbl = memnew(Label);
    lbl->set_text("Loading application...");
    lbl->set_horizontal_alignment(HORIZONTAL_ALIGNMENT_CENTER);
    vbox->add_child(lbl);

    auto* progress = memnew(ProgressBar);
    progress->set_min(0.0);
    progress->set_max(1.0);
    progress->set_value(0.0);
    progress->set_custom_minimum_size(Vector2(400, 20));
    vbox->add_child(progress);

    add_child(root_ctrl);
    content_root_    = root_ctrl;
    splash_progress_ = progress;
}

void ForgeRunnerNativeMain::update_download_progress(std::size_t done, std::size_t total,
                                                      const std::string& /*current_file*/) {
    if (!splash_progress_) return;
    const double ratio = (total > 0) ? (static_cast<double>(done) / static_cast<double>(total))
                                      : 0.0;
    splash_progress_->set_value(ratio);
}

// ---------------------------------------------------------------------------
// Manifest download
// ---------------------------------------------------------------------------

void ForgeRunnerNativeMain::start_manifest_download(const std::string& manifest_url) {
    manifest_url_    = manifest_url;
    per_manifest_dir_ = per_manifest_cache_root(manifest_url);

    http_request_ = memnew(HTTPRequest);
    add_child(http_request_);
    http_request_->connect("request_completed",
        callable_mp(this, &ForgeRunnerNativeMain::on_manifest_downloaded));

    if (http_request_->request(String(manifest_url.c_str())) != Error::OK) {
        show_error("Failed to initiate manifest download: " + manifest_url);
        http_request_->queue_free();
        http_request_ = nullptr;
    }
    UtilityFunctions::print(String(("[ForgeRunner.Native] Downloading manifest: " +
                                    manifest_url).c_str()));
}

void ForgeRunnerNativeMain::on_manifest_downloaded(int result, int code,
                                                    PackedStringArray /*headers*/,
                                                    PackedByteArray body) {
    if (http_request_) {
        http_request_->queue_free();
        http_request_ = nullptr;
    }

    if (result != HTTPRequest::RESULT_SUCCESS || (code != 200 && code != 0)) {
        // Try cached version if available
        std::string cached_sha256, cached_entry;
        std::unordered_map<std::string, std::string> cached_hashes;
        if (load_manifest_meta(per_manifest_dir_, cached_sha256, cached_entry, cached_hashes)) {
            const std::string cached_entry_path =
                per_manifest_dir_ + "/files/" + cached_entry;
            if (fs::exists(cached_entry_path)) {
                UtilityFunctions::push_warning(String(
                    ("[ForgeRunner.Native] Manifest download failed (HTTP " +
                     std::to_string(code) + "), using cached version.").c_str()));
                show_sml(cached_entry_path);
                return;
            }
        }
        show_error("Manifest download failed (HTTP " + std::to_string(code) +
                   "): " + manifest_url_);
        return;
    }

    // Convert body to string for parsing
    std::string content(body.size(), '\0');
    for (int64_t i = 0; i < body.size(); ++i)
        content[static_cast<std::size_t>(i)] = static_cast<char>(body[i]);

    // Compute manifest SHA-256 for delta detection
    const std::string manifest_sha256 = forge::AssetCache::sha256_of(
        std::vector<uint8_t>(content.begin(), content.end()));

    // Check if manifest is unchanged and all cached files still exist
    std::string cached_sha256, cached_entry;
    std::unordered_map<std::string, std::string> cached_hashes;
    if (load_manifest_meta(per_manifest_dir_, cached_sha256, cached_entry, cached_hashes)) {
        if (cached_sha256 == manifest_sha256) {
            const std::string entry_path = per_manifest_dir_ + "/files/" + cached_entry;
            if (fs::exists(entry_path)) {
                UtilityFunctions::print("[ForgeRunner.Native] Manifest unchanged, using cache.");
                show_sml(entry_path);
                return;
            }
        }
    }

    // Parse manifest with smlcore
    smlcore::Document doc;
    try {
        doc = smlcore::parse_document(content);
    } catch (const std::exception& ex) {
        show_error(std::string("Manifest parse error: ") + ex.what());
        return;
    }
    if (doc.roots.empty()) {
        show_error("Empty manifest: " + manifest_url_);
        return;
    }

    auto lower_str = [](std::string s) {
        std::transform(s.begin(), s.end(), s.begin(),
                       [](unsigned char c){ return std::tolower(c); });
        return s;
    };

    if (lower_str(doc.roots[0].name) != "manifest") {
        show_error("Manifest root must be 'Manifest', got '" + doc.roots[0].name + "'");
        return;
    }

    const auto& root = doc.roots[0];
    std::string entry_point = "app.sml";
    std::string base_url;

    for (const auto& prop : root.properties) {
        const auto k = lower_str(prop.name);
        if      (k == "entry" || k == "entrypoint") entry_point = prop.value;
        else if (k == "baseurl")                    base_url    = prop.value;
    }

    manifest_entry_relative_ = entry_point;
    download_queue_.clear();

    for (const auto& child : root.children) {
        if (lower_str(child.name) != "files") continue;
        for (const auto& file_node : child.children) {
            if (lower_str(file_node.name) != "file") continue;

            ManifestAsset asset;
            for (const auto& prop : file_node.properties) {
                const auto k = lower_str(prop.name);
                if      (k == "path") asset.path   = prop.value;
                else if (k == "hash") asset.sha256  = prop.value;
                else if (k == "url")  asset.url     = prop.value;
            }
            if (asset.url.empty()) asset.url = asset.path;
            asset.url  = resolve_asset_url(manifest_url_, base_url, asset.url);
            asset.path = normalize_asset_path(asset.path);

            if (!asset.path.empty() && !asset.sha256.empty())
                download_queue_.push_back(asset);
        }
    }

    // Store manifest content and sha256 for later metadata save
    // (saved after all assets are done in on_all_assets_ready)
    // Keep cached_hashes around to skip already-valid files.
    // We store sha256 in a temporary member — reuse per_manifest_dir_ as key.
    // Pass manifest_sha256 through the static save at the end.
    // Store it as a member for use in on_all_assets_ready.
    manifest_sha256_ = manifest_sha256;

    cached_asset_hashes_ = cached_hashes;  // for delta skip in start_next_asset_download
    download_queue_idx_ = 0;
    download_retry_     = 0;

    if (download_queue_.empty()) {
        // No assets to download — go straight to entry point
        const std::string entry_path = per_manifest_dir_ + "/files/" + manifest_entry_relative_;
        if (fs::exists(entry_path)) {
            show_sml(entry_path);
        } else {
            show_error("Manifest has no downloadable assets and entry point is not cached.");
        }
        return;
    }

    update_download_progress(0, download_queue_.size(), "");
    start_next_asset_download();
}

// ---------------------------------------------------------------------------
// Asset download loop
// ---------------------------------------------------------------------------

void ForgeRunnerNativeMain::start_next_asset_download() {
    // Skip assets whose cached file matches the expected hash from metadata
    while (download_queue_idx_ < download_queue_.size()) {
        const auto& asset    = download_queue_[download_queue_idx_];
        const auto  dst_path = per_manifest_dir_ + "/files/" + asset.path;

        if (fs::exists(dst_path)) {
            const auto meta_it = cached_asset_hashes_.find(asset.path);
            // Hashes stored in metadata may carry "sha256:" prefix — normalize both
            auto strip_prefix = [](const std::string& h) {
                const std::string pfx = "sha256:";
                return (h.size() > pfx.size() && h.substr(0, pfx.size()) == pfx)
                       ? h.substr(pfx.size()) : h;
            };
            const bool hash_ok = (meta_it != cached_asset_hashes_.end()) &&
                                 (strip_prefix(meta_it->second) == strip_prefix(asset.sha256));
            if (hash_ok) {
                ++download_queue_idx_;
                update_download_progress(download_queue_idx_, download_queue_.size(), "");
                continue;
            }
        }

        // Need to download
        update_download_progress(download_queue_idx_, download_queue_.size(), asset.path);

        http_request_ = memnew(HTTPRequest);
        add_child(http_request_);
        http_request_->connect("request_completed",
            callable_mp(this, &ForgeRunnerNativeMain::on_asset_downloaded));

        if (http_request_->request(String(asset.url.c_str())) != Error::OK) {
            UtilityFunctions::push_error(String(
                ("[ForgeRunner.Native] Failed to start asset download: " + asset.url).c_str()));
            http_request_->queue_free();
            http_request_ = nullptr;
            ++download_queue_idx_;
            download_retry_ = 0;
            continue;
        }

        UtilityFunctions::print(String(
            ("[ForgeRunner.Native] Downloading asset: " + asset.path).c_str()));
        return;  // wait for on_asset_downloaded callback
    }

    // All assets processed
    on_all_assets_ready();
}

void ForgeRunnerNativeMain::on_asset_downloaded(int result, int code,
                                                 PackedStringArray /*headers*/,
                                                 PackedByteArray body) {
    if (http_request_) {
        http_request_->queue_free();
        http_request_ = nullptr;
    }

    const auto& asset    = download_queue_[download_queue_idx_];
    const auto  dst_path = per_manifest_dir_ + "/files/" + asset.path;

    if (result == HTTPRequest::RESULT_SUCCESS && (code == 200 || code == 206)) {
        const std::vector<uint8_t> data(body.ptr(), body.ptr() + body.size());

        // Verify hash if manifest provided one
        if (!asset.sha256.empty()) {
            auto strip = [](const std::string& h) {
                const std::string pfx = "sha256:";
                return (h.size() > pfx.size() && h.substr(0, pfx.size()) == pfx)
                       ? h.substr(pfx.size()) : h;
            };
            const auto actual = forge::AssetCache::sha256_of(data);
            if (strip(actual) != strip(asset.sha256)) {
                UtilityFunctions::push_warning(String(
                    ("[ForgeRunner.Native] Hash mismatch for " + asset.path +
                     " (expected " + strip(asset.sha256) +
                     " got " + actual + ")").c_str()));
                // Treat as failed download → retry path below
                if (download_retry_ < 2) {
                    ++download_retry_;
                    http_request_ = memnew(HTTPRequest);
                    add_child(http_request_);
                    http_request_->connect("request_completed",
                        callable_mp(this, &ForgeRunnerNativeMain::on_asset_downloaded));
                    http_request_->request(String(asset.url.c_str()));
                    return;
                }
                UtilityFunctions::push_error(String(
                    ("[ForgeRunner.Native] Hash mismatch after retries, skipping: " +
                     asset.path).c_str()));
                download_retry_ = 0;
                ++download_queue_idx_;
                start_next_asset_download();
                return;
            }
        }

        // Save asset at its original relative path (preserves directory structure)
        if (!save_file_atomic(dst_path, data.data(), data.size())) {
            UtilityFunctions::push_warning(String(
                ("[ForgeRunner.Native] Failed to cache asset: " + asset.path).c_str()));
        }
        download_retry_ = 0;
        ++download_queue_idx_;
    } else if (download_retry_ < 2) {
        // One retry on failure
        ++download_retry_;
        UtilityFunctions::push_warning(String(
            ("[ForgeRunner.Native] Asset download failed (HTTP " + std::to_string(code) +
             "), retry " + std::to_string(download_retry_) + ": " + asset.url).c_str()));

        http_request_ = memnew(HTTPRequest);
        add_child(http_request_);
        http_request_->connect("request_completed",
            callable_mp(this, &ForgeRunnerNativeMain::on_asset_downloaded));
        http_request_->request(String(asset.url.c_str()));
        return;
    } else {
        UtilityFunctions::push_error(String(
            ("[ForgeRunner.Native] Asset download failed after retries: " + asset.url).c_str()));
        download_retry_ = 0;
        ++download_queue_idx_;
    }

    update_download_progress(download_queue_idx_, download_queue_.size(),
                             download_queue_idx_ < download_queue_.size()
                                 ? download_queue_[download_queue_idx_].path : "");
    start_next_asset_download();
}

void ForgeRunnerNativeMain::on_all_assets_ready() {
    // Save manifest metadata for delta detection on next run
    save_manifest_meta(per_manifest_dir_, manifest_sha256_,
                       manifest_entry_relative_, download_queue_);

    const std::string entry_path =
        per_manifest_dir_ + "/files/" + manifest_entry_relative_;

    if (!fs::exists(entry_path)) {
        show_error("Entry point not found after download: " + manifest_entry_relative_);
        return;
    }

    UtilityFunctions::print(String(
        ("[ForgeRunner.Native] All assets ready, showing: " + entry_path).c_str()));
    show_sml(entry_path);
}
