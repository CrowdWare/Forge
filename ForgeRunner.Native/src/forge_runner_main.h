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

#pragma once

#include "forge_asset_cache.h"
#include "forge_sms_bridge.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include <godot_cpp/classes/canvas_layer.hpp>
#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/string.hpp>

// Forward declarations - full headers included in forge_runner_main.cpp
namespace godot {
class Control;
class HTTPRequest;
class FileDialog;
class ProgressBar;
class Timer;
class TreeItem;
class VBoxContainer;
}

class ForgeRunnerNativeMain : public godot::Node {
    GDCLASS(ForgeRunnerNativeMain, godot::Node);

protected:
    static void _bind_methods();

public:
    void _ready() override;
    void _process(double delta) override;

    // SMS event callbacks - called via Godot signal connections.
    // Signal args come first; bound args (object_id, event_name) are appended.
    void on_sms_event(godot::String object_id, godot::String event_name);
    void on_sms_bool_event(bool value, godot::String object_id, godot::String event_name);
    void on_sms_text_event(godot::String text, godot::String object_id, godot::String event_name);
    void on_sms_value_event(double value, godot::String object_id, godot::String event_name);
    void on_sms_item_event(int index, godot::String object_id, godot::String event_name);
    void on_sms_item_text_event(int index, godot::String text, godot::String object_id, godot::String event_name);
    void on_sms_popup_item_selected(int id, godot::String object_id, godot::String event_name);
    void on_sms_popup_item_selected_by_index(int index, godot::String object_id, godot::String event_name);
    void on_sms_file_dialog_selected(godot::String path, godot::String callback_name, int64_t dialog_id);
    void on_sms_file_dialog_canceled(godot::String callback_name, int64_t dialog_id);
    void open_sms_file_dialog(const std::string& callback_name, const std::string& filter, bool save_mode);
    void on_sms_tree_button_clicked(godot::TreeItem* item, int column, int id, int mouse_button_index, godot::String object_id, godot::String event_name);

    // Viewport resize -> mainWindow.orientationChanged SMS event
    void on_viewport_size_changed();

private:
    // ----- SML / Splash state -----
    godot::Control* content_root_    = nullptr;
    godot::Timer*   splash_timer_    = nullptr;
    std::string     splash_next_path_;

    // ----- Dev hot-reload (HTTP only) -----
    godot::Timer*      dev_poll_timer_   = nullptr;
    bool               is_polling_       = false;
    void               start_dev_poll_timer();
    void               on_dev_poll_timeout();

    // ----- SMS Guru Meditation (error overlay) -----
    godot::CanvasLayer* sms_toast_layer_ = nullptr;
    void                show_sms_guru(const std::string& msg);
    void                on_sms_guru_input(godot::Ref<godot::InputEvent> event);
    void                on_sms_guru_dismissed();

    // ----- SMS state -----
    forge::SmsBridge sms_bridge_;
    std::int64_t     sms_session_ = -1;

    // ----- Manifest download state -----
    struct ManifestAsset {
        std::string path;    ///< relative path in app, e.g. "app.sml"
        std::string url;     ///< absolute download URL
        std::string sha256;  ///< expected hash (may carry "sha256:" prefix)
    };

    forge::AssetCache           asset_cache_;
    std::string                 manifest_url_;
    std::string                 manifest_sha256_;       ///< SHA-256 of last downloaded manifest
    std::string                 per_manifest_dir_;      ///< cache root for this manifest
    std::string                 manifest_entry_relative_; ///< e.g. "app.sml"
    std::vector<ManifestAsset>              download_queue_;
    std::unordered_map<std::string,
                       std::string>        cached_asset_hashes_; ///< path→sha256 from last metadata
    size_t                                 download_queue_idx_ = 0;
    int                                    download_retry_     = 0;
    godot::HTTPRequest*         http_request_       = nullptr;
    godot::ProgressBar*         splash_progress_    = nullptr;
    std::unordered_map<std::string, std::unordered_map<int, std::string>> popup_item_id_map_;
    std::unordered_map<std::string, std::unordered_map<int, int>> popup_item_index_to_id_map_;
    std::unordered_map<std::int64_t, godot::FileDialog*> open_file_dialogs_;

    // ----- Helpers -----

    static std::string file_url_to_path(const std::string& url);
    static std::string load_file(const std::string& path);

    void show_sml(const std::string& path);
    void clear_content();
    void show_error(const std::string& msg);
    void on_splash_timeout();
    void start_sms(const std::string& sml_path, const std::string& root_name, const std::string& root_id);
    void bind_popup_menu_events(godot::Node* node);
    void stop_sms();

    // ----- HTTP / Manifest download -----

    static bool        is_http_url(const std::string& url);
    static std::string per_manifest_cache_root(const std::string& manifest_url);
    static std::string resolve_asset_url(const std::string& manifest_url,
                                         const std::string& base_url,
                                         const std::string& raw_url);
    static std::string normalize_asset_path(const std::string& path);
    static bool        save_file_atomic(const std::string& path,
                                        const void* data, std::size_t size);
    static bool        load_manifest_meta(
                            const std::string& cache_root,
                            std::string& out_manifest_sha256,
                            std::string& out_entry_point,
                            std::unordered_map<std::string, std::string>& out_hashes);
    static void        save_manifest_meta(
                            const std::string& cache_root,
                            const std::string& manifest_sha256,
                            const std::string& entry_point,
                            const std::vector<ManifestAsset>& assets);

    void show_loading_screen();
    void update_download_progress(std::size_t done, std::size_t total,
                                  const std::string& current_file);
    void start_manifest_download(const std::string& manifest_url);
    void on_manifest_downloaded(int result, int code,
                                godot::PackedStringArray headers,
                                godot::PackedByteArray   body);
    void start_next_asset_download();
    void on_asset_downloaded(int result, int code,
                             godot::PackedStringArray headers,
                             godot::PackedByteArray   body);
    void on_all_assets_ready();
};
