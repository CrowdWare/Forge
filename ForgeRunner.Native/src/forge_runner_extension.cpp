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
#include "forge_sms_bridge.h"
#include "forge_markdown.h"
#include "forge_path_resolver.h"
#include <sml_document.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <iomanip>
#include <limits>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/box_mesh.hpp>
#include <godot_cpp/classes/code_edit.hpp>
#include <godot_cpp/classes/color_rect.hpp>
#include <godot_cpp/classes/camera3d.hpp>
#include <godot_cpp/classes/cylinder_mesh.hpp>
#include <godot_cpp/classes/directional_light3d.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/gltf_document.hpp>
#include <godot_cpp/classes/gltf_state.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/plane_mesh.hpp>
#include <godot_cpp/classes/shader.hpp>
#include <godot_cpp/classes/shader_material.hpp>
#include <godot_cpp/classes/rich_text_label.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/skeleton3d.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/classes/sub_viewport.hpp>
#include <godot_cpp/classes/sphere_mesh.hpp>
#include <godot_cpp/classes/text_server.hpp>
#include <godot_cpp/classes/texture_rect.hpp>
#include <godot_cpp/classes/tree.hpp>
#include <godot_cpp/classes/tree_item.hpp>
#include <godot_cpp/classes/container.hpp>
#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/h_box_container.hpp>
#include <godot_cpp/classes/h_scroll_bar.hpp>
#include <godot_cpp/classes/h_separator.hpp>
#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/classes/input_event_mouse_motion.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/line_edit.hpp>
#include <godot_cpp/classes/menu_button.hpp>
#include <godot_cpp/classes/material.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/panel_container.hpp>
#include <godot_cpp/classes/popup_menu.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/style_box_flat.hpp>
#include <godot_cpp/classes/sub_viewport_container.hpp>
#include <godot_cpp/classes/viewport_texture.hpp>
#include <godot_cpp/classes/tab_bar.hpp>
#include <godot_cpp/classes/tab_container.hpp>
#include <godot_cpp/classes/display_server.hpp>
#include <godot_cpp/classes/font.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/input_event_key.hpp>
#include <godot_cpp/classes/immediate_mesh.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/v_box_container.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/object.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/quaternion.hpp>
#include <godot_cpp/variant/rect2.hpp>

#if defined(__ANDROID__)
#include <jni.h>
#endif

#include <vector>

using namespace godot;

// ---------------------------------------------------------------------------
// ForgeDockingContainerControl
// ---------------------------------------------------------------------------

class ForgeDockingContainerControl : public TabContainer {
    GDCLASS(ForgeDockingContainerControl, TabContainer);

    struct FloatEntry {
        Window*  window  = nullptr;
        Control* content = nullptr;
        String   title;
    };

    // Dock grid columns: [top_side, bottom_side] × 4 + center
    static constexpr const char* GRID_COLS[4][2] = {
        {"farleft",  "farleftbottom"},
        {"left",     "leftbottom"},
        {"right",    "rightbottom"},
        {"farright", "farrightbottom"},
    };
    // All 8 side positions + center — always shown, disabled when no target
    static constexpr const char* ALL_POSITIONS[9] = {
        "farleft", "farleftbottom", "left", "leftbottom",
        "center",
        "right", "rightbottom", "farright", "farrightbottom"
    };

protected:
    static void _bind_methods() {
        ClassDB::bind_method(D_METHOD("set_dock_side",      "side"),  &ForgeDockingContainerControl::set_dock_side);
        ClassDB::bind_method(D_METHOD("get_dock_side"),               &ForgeDockingContainerControl::get_dock_side);
        ClassDB::bind_method(D_METHOD("set_fixed_width",    "value"), &ForgeDockingContainerControl::set_fixed_width);
        ClassDB::bind_method(D_METHOD("get_fixed_width"),             &ForgeDockingContainerControl::get_fixed_width);
        ClassDB::bind_method(D_METHOD("get_base_fixed_width"),        &ForgeDockingContainerControl::get_base_fixed_width);
        ClassDB::bind_method(D_METHOD("set_fixed_height",   "value"), &ForgeDockingContainerControl::set_fixed_height);
        ClassDB::bind_method(D_METHOD("get_fixed_height"),            &ForgeDockingContainerControl::get_fixed_height);
        ClassDB::bind_method(D_METHOD("set_height_percent", "value"), &ForgeDockingContainerControl::set_height_percent);
        ClassDB::bind_method(D_METHOD("get_height_percent"),          &ForgeDockingContainerControl::get_height_percent);
        ClassDB::bind_method(D_METHOD("set_flex",           "value"), &ForgeDockingContainerControl::set_flex);
        ClassDB::bind_method(D_METHOD("is_flex"),                     &ForgeDockingContainerControl::is_flex);
        ClassDB::bind_method(D_METHOD("set_collapsed",      "value"), &ForgeDockingContainerControl::set_collapsed);
        ClassDB::bind_method(D_METHOD("is_collapsed"),                &ForgeDockingContainerControl::is_collapsed);
        ClassDB::bind_method(D_METHOD("_on_about_to_popup"),          &ForgeDockingContainerControl::_on_about_to_popup);
        ClassDB::bind_method(D_METHOD("_return_from_float", "win_id"),&ForgeDockingContainerControl::_return_from_float);
        ClassDB::bind_method(D_METHOD("_on_dock_btn_pressed", "pos"), &ForgeDockingContainerControl::_on_dock_btn_pressed);
        ClassDB::bind_method(D_METHOD("_on_float_pressed"),           &ForgeDockingContainerControl::_on_float_pressed);
        ClassDB::bind_method(D_METHOD("_on_close_pressed"),           &ForgeDockingContainerControl::_on_close_pressed);
        ClassDB::bind_method(D_METHOD("_on_overlay_input", "event"),  &ForgeDockingContainerControl::_on_overlay_input);
    }

public:
    // -----------------------------------------------------------------------
    // Lifecycle
    // -----------------------------------------------------------------------
    void _ready() override {
        // Prevent dock content from drawing outside its assigned column rect.
        set_clip_contents(true);
        // Use set_popup() to get the built-in ⋮ button in the tab bar.
        // We suppress the actual popup and show our custom dialog instead.
        popup_ = memnew(PopupMenu);
        add_child(popup_);
        set_popup(popup_);
        popup_->connect("about_to_popup",
            callable_mp(this, &ForgeDockingContainerControl::_on_about_to_popup));
    }

    void _on_about_to_popup() {
        popup_->call_deferred("hide");  // deferred: avoids swallowing the next click
        _ensure_dialog();
        _update_dock_buttons();

        Window* root = get_tree() ? get_tree()->get_root() : nullptr;
        if (!root) return;

        if (!overlay_->get_parent()) root->add_child(overlay_);
        if (!dialog_->get_parent())  root->add_child(dialog_);

        overlay_->set_z_index(1999);
        overlay_->set_visible(true);
        dialog_->set_z_index(2000);
        dialog_->set_visible(true);

        // Position just below the tab bar, aligned to its right edge
        TabBar* tb  = get_tab_bar();
        Rect2   tbr = tb ? tb->get_global_rect() : get_global_rect();
        float   dw  = 210.f;
        float   dh  = 220.f;
        float   dx  = tbr.position.x + tbr.size.x - dw;
        float   dy  = tbr.position.y + tbr.size.y + 4.f;

        Rect2 vp = get_viewport()->get_visible_rect();
        if (dx < vp.position.x)                          dx = vp.position.x;
        if (dx + dw > vp.position.x + vp.size.x)        dx = vp.position.x + vp.size.x - dw;
        if (dy + dh > vp.position.y + vp.size.y)        dy = vp.position.y + vp.size.y - dh;

        dialog_->set_global_position(Vector2(dx, dy));
    }

    void _hide_dialog() {
        if (overlay_) overlay_->set_visible(false);
        if (dialog_)  dialog_->set_visible(false);
    }

    // -----------------------------------------------------------------------
    // Dialog button handlers
    // -----------------------------------------------------------------------
    void _on_dock_btn_pressed(String pos) {
        _hide_dialog();
        ForgeDockingContainerControl* target = _find_sibling(pos);
        if (target) _move_tab_to(target);
    }

    void _on_float_pressed() {
        _hide_dialog();
        _make_floating();
    }

    void _on_close_pressed() {
        _hide_dialog();
        _close_panel();
    }

    void _on_overlay_input(Ref<InputEvent> event) {
        Ref<InputEventMouseButton> mb = event;
        if (mb.is_valid() && mb->is_pressed())
            _hide_dialog();
    }

    // -----------------------------------------------------------------------
    // Move current tab to another DockingContainer
    // -----------------------------------------------------------------------
    void _move_tab_to(ForgeDockingContainerControl* target) {
        if (!target) return;
        int idx = get_current_tab();
        if (idx < 0 || idx >= get_tab_count()) return;

        String   title   = get_tab_title(idx);
        Control* content = get_tab_control(idx);
        if (!content) return;

        // Make target visible BEFORE add_child so TabContainer lays out immediately
        target->set_visible(true);

        remove_child(content);
        // TabContainer hides non-current tabs on remove — restore before re-adding
        content->set_visible(true);
        target->add_child(content);
        target->set_tab_title(target->get_tab_count() - 1, title);
        target->set_current_tab(target->get_tab_count() - 1);
        content->set_anchors_and_offsets_preset(Control::PRESET_FULL_RECT);

        // Collapse source if now empty
        if (get_tab_count() == 0) set_visible(false);
        // Immediate host reflow so the new column gets its correct size right away
        auto* host = Object::cast_to<Container>(get_parent());
        if (host) host->notification(Container::NOTIFICATION_SORT_CHILDREN);
    }

    // -----------------------------------------------------------------------
    // Make Floating — pops current tab into a native OS window
    // -----------------------------------------------------------------------
    void _make_floating() {
        int idx = get_current_tab();
        if (idx < 0 || idx >= get_tab_count()) return;

        String   title      = get_tab_title(idx);
        Control* content    = get_tab_control(idx);
        if (!content) return;

        // Capture size + screen position BEFORE removing the tab (layout changes after)
        Rect2   global_rect = get_global_rect();
        Vector2 screen_pos  = get_screen_position();

        remove_child(content);

        Window* win = memnew(Window);
        win->set_title(title);
        win->set_initial_position(Window::WINDOW_INITIAL_POSITION_ABSOLUTE);
        win->set_position(Vector2i((int)screen_pos.x, (int)screen_pos.y));
        win->set_size(Vector2i(
            (int)global_rect.size.x > 220 ? (int)global_rect.size.x : 220,
            (int)global_rect.size.y > 140 ? (int)global_rect.size.y : 140));
        content->set_anchors_and_offsets_preset(Control::PRESET_FULL_RECT);
        win->add_child(content);

        SceneTree* tree = get_tree();
        if (!tree) { win->queue_free(); add_child(content); return; }

        Window* root = tree->get_root();
        if (root->is_embedding_subwindows())
            root->set_embedding_subwindows(false);

        // get_window() returns the Window node this control lives in — that is
        // _mainAppWindow, the real visible app window.  Making our float window
        // transient to it establishes the correct macOS parent/child relationship.
        Window* owner = get_window();
        if (!owner) owner = root;

        float_entries_.push_back({win, content, title});
        win->connect("close_requested",
            callable_mp(this, &ForgeDockingContainerControl::_return_from_float)
                .bind(Variant(static_cast<int64_t>(win->get_instance_id()))));

        win->set_transient(true);
        owner->add_child(win);
        win->show();
    }

    // -----------------------------------------------------------------------
    // Return floating tab back to this container
    // -----------------------------------------------------------------------
    void _return_from_float(int64_t win_id) {
        for (int i = 0; i < (int)float_entries_.size(); ++i) {
            FloatEntry& fe = float_entries_[i];
            if (!fe.window) continue;
            if (static_cast<int64_t>(fe.window->get_instance_id()) != win_id) continue;

            Window*  win     = fe.window;
            Control* content = fe.content;
            String   title   = fe.title;

            if (content->get_parent() == win) win->remove_child(content);

            add_child(content);
            set_tab_title(get_tab_count() - 1, title);
            set_current_tab(get_tab_count() - 1);
            content->set_anchors_and_offsets_preset(Control::PRESET_FULL_RECT);

            float_entries_.erase(float_entries_.begin() + i);
            win->queue_free();
            return;
        }
    }

    // -----------------------------------------------------------------------
    // Close current tab
    // -----------------------------------------------------------------------
    void _close_panel() {
        int idx = get_current_tab();
        if (idx < 0 || idx >= get_tab_count()) return;
        Control* content = get_tab_control(idx);
        if (!content) return;
        remove_child(content);
        content->set_visible(false);
        if (get_tab_count() == 0) {
            set_visible(false);
            auto* host = Object::cast_to<Container>(get_parent());
            if (host) host->notification(Container::NOTIFICATION_SORT_CHILDREN);
        }
    }

    // -----------------------------------------------------------------------
    // Properties
    // -----------------------------------------------------------------------
    void   set_dock_side(const String& side) { dock_side_ = side.to_lower(); }
    String get_dock_side()    const { return dock_side_; }

    void   set_fixed_width(double v)  {
        fixed_width_ = static_cast<float>(v);
        if (fixed_width_ > 0.f && base_fixed_width_ <= 0.f) base_fixed_width_ = fixed_width_;
    }
    double get_fixed_width()    const { return fixed_width_; }
    double get_base_fixed_width() const { return base_fixed_width_; }

    void   set_fixed_height(double v) { fixed_height_ = static_cast<float>(v); }
    double get_fixed_height()   const { return fixed_height_; }

    void   set_height_percent(double v) { height_percent_ = static_cast<float>(v); }
    double get_height_percent()   const { return height_percent_; }

    void set_flex(bool v) { flex_ = v; }
    bool is_flex()  const { return flex_; }
    void set_collapsed(bool v) {
        collapsed_ = v;
        set_visible(!collapsed_);
    }
    bool is_collapsed() const { return collapsed_; }

private:
    // -----------------------------------------------------------------------
    // Lazy dialog construction
    // -----------------------------------------------------------------------
    void _ensure_dialog() {
        if (dialog_) return;

        // --- Fullscreen overlay (click-outside-to-close) ---
        overlay_ = memnew(Control);
        overlay_->set_name("DockOverlay");
        overlay_->set_anchors_preset(Control::PRESET_FULL_RECT);
        overlay_->set_mouse_filter(Control::MOUSE_FILTER_STOP);
        overlay_->set_visible(false);
        overlay_->connect("gui_input",
            callable_mp(this, &ForgeDockingContainerControl::_on_overlay_input));

        // --- Dialog panel ---
        PanelContainer* dlg = memnew(PanelContainer);
        dlg->set_name("DockDialog");
        dlg->set_custom_minimum_size(Vector2(210, 0));
        dlg->set_visible(false);
        dlg->set_mouse_filter(Control::MOUSE_FILTER_STOP);

        Ref<StyleBoxFlat> dlg_style; dlg_style.instantiate();
        dlg_style->set_bg_color(Color(0.14f, 0.15f, 0.18f, 0.98f));
        dlg_style->set_border_width_all(1);
        dlg_style->set_border_color(Color(0.26f, 0.30f, 0.36f, 1.f));
        dlg_style->set_corner_radius_all(6);
        dlg_style->set_shadow_color(Color(0.f, 0.f, 0.f, 0.40f));
        dlg_style->set_shadow_size(8);
        dlg_style->set_shadow_offset(Vector2(0, 2));
        dlg_style->set_content_margin_all(8.f);
        dlg->add_theme_stylebox_override("panel", dlg_style);

        // Root VBox
        VBoxContainer* vbox = memnew(VBoxContainer);
        vbox->add_theme_constant_override("separation", 6);
        vbox->set_h_size_flags(Control::SIZE_EXPAND_FILL);

        // Header label
        Label* title_label = memnew(Label);
        title_label->set_text("Dock Position");
        title_label->set_horizontal_alignment(HORIZONTAL_ALIGNMENT_CENTER);
        title_label->set_h_size_flags(Control::SIZE_EXPAND_FILL);
        title_label->add_theme_color_override("font_color", Color(0.87f, 0.91f, 0.98f, 1.f));
        vbox->add_child(title_label);

        // Dock grid: columns + center
        HBoxContainer* grid = memnew(HBoxContainer);
        grid->set_h_size_flags(Control::SIZE_SHRINK_CENTER);
        grid->set_v_size_flags(Control::SIZE_SHRINK_CENTER);
        grid->add_theme_constant_override("separation", 3);

        // Left two columns
        grid->add_child(_make_dock_column(GRID_COLS[0][0], GRID_COLS[0][1]));
        grid->add_child(_make_dock_column(GRID_COLS[1][0], GRID_COLS[1][1]));
        // Center slot
        grid->add_child(_make_slot_btn("center", true));
        // Right two columns
        grid->add_child(_make_dock_column(GRID_COLS[2][0], GRID_COLS[2][1]));
        grid->add_child(_make_dock_column(GRID_COLS[3][0], GRID_COLS[3][1]));

        vbox->add_child(grid);

        // Separator
        HSeparator* sep = memnew(HSeparator);
        sep->set_h_size_flags(Control::SIZE_EXPAND_FILL);
        vbox->add_child(sep);

        // Action buttons
        Button* float_btn = memnew(Button);
        float_btn->set_text("  Make Floating");
        float_btn->set_flat(false);
        float_btn->set_text_alignment(HORIZONTAL_ALIGNMENT_LEFT);
        float_btn->set_h_size_flags(Control::SIZE_EXPAND_FILL);
        float_btn->connect("pressed",
            callable_mp(this, &ForgeDockingContainerControl::_on_float_pressed));
        vbox->add_child(float_btn);

        Button* close_btn = memnew(Button);
        close_btn->set_text("  Close Panel");
        close_btn->set_flat(false);
        close_btn->set_text_alignment(HORIZONTAL_ALIGNMENT_LEFT);
        close_btn->set_h_size_flags(Control::SIZE_EXPAND_FILL);
        close_btn->connect("pressed",
            callable_mp(this, &ForgeDockingContainerControl::_on_close_pressed));
        vbox->add_child(close_btn);

        dlg->add_child(vbox);
        dialog_ = dlg;
    }

    VBoxContainer* _make_dock_column(const char* top_pos, const char* bot_pos) {
        VBoxContainer* col = memnew(VBoxContainer);
        col->set_h_size_flags(Control::SIZE_SHRINK_CENTER);
        col->set_v_size_flags(Control::SIZE_EXPAND_FILL);
        col->set_custom_minimum_size(Vector2(36, 0));
        col->add_theme_constant_override("separation", 3);
        col->add_child(_make_slot_btn(top_pos, false));
        col->add_child(_make_slot_btn(bot_pos, false));
        return col;
    }

    Button* _make_slot_btn(const char* pos, bool center) {
        Button* btn = memnew(Button);
        btn->set_text("");
        btn->set_flat(false);
        btn->set_tooltip_text(String(pos));
        if (center) {
            btn->set_custom_minimum_size(Vector2(44, 0));
            btn->set_h_size_flags(Control::SIZE_EXPAND_FILL);
            btn->set_v_size_flags(Control::SIZE_EXPAND_FILL);
        } else {
            btn->set_custom_minimum_size(Vector2(34, 26));
            btn->set_h_size_flags(Control::SIZE_SHRINK_CENTER);
            btn->set_v_size_flags(Control::SIZE_EXPAND_FILL);
        }

        auto mk = [center](float r, float g, float b, float a = 1.f) {
            Ref<StyleBoxFlat> s; s.instantiate();
            s->set_bg_color(Color(r, g, b, a));
            s->set_corner_radius_all(5);
            s->set_border_width_all(0);
            return s;
        };
        if (center) {
            btn->add_theme_stylebox_override("normal",   mk(0.34f, 0.36f, 0.42f));
            btn->add_theme_stylebox_override("hover",    mk(0.40f, 0.42f, 0.50f));
            btn->add_theme_stylebox_override("pressed",  mk(0.30f, 0.44f, 0.68f));
            btn->add_theme_stylebox_override("disabled", mk(0.26f, 0.28f, 0.34f));
        } else {
            btn->add_theme_stylebox_override("normal",   mk(0.24f, 0.24f, 0.28f));
            btn->add_theme_stylebox_override("hover",    mk(0.30f, 0.32f, 0.38f));
            btn->add_theme_stylebox_override("pressed",  mk(0.20f, 0.34f, 0.56f));
            btn->add_theme_stylebox_override("disabled", mk(0.28f, 0.30f, 0.34f, 0.95f));
        }
        btn->add_theme_color_override("font_color", Color(0.96f, 0.96f, 0.98f, 1.f));

        dock_btns_[pos] = btn;
        btn->connect("pressed",
            callable_mp(this, &ForgeDockingContainerControl::_on_dock_btn_pressed)
                .bind(Variant(String(pos))));
        return btn;
    }

    void _update_dock_buttons() {
        bool  can_move = get_drag_to_rearrange_enabled();
        int   rg       = get_tabs_rearrange_group();
        Node* parent   = get_parent();

        for (auto& [pos, btn] : dock_btns_) {
            bool available = false;
            if (can_move && parent) {
                int n = parent->get_child_count();
                for (int i = 0; i < n; ++i) {
                    auto* s = Object::cast_to<ForgeDockingContainerControl>(parent->get_child(i));
                    if (!s || s == this) continue;
                    if (!s->get_drag_to_rearrange_enabled()) continue;
                    if (s->get_tabs_rearrange_group() != rg) continue;
                    if (s->get_dock_side() == String(pos.c_str())) { available = true; break; }
                }
            }
            // All buttons always visible; disabled when no target exists
            btn->set_visible(true);
            btn->set_disabled(!available);
        }
    }

    ForgeDockingContainerControl* _find_sibling(const String& side) {
        Node* parent = get_parent();
        if (!parent) return nullptr;
        int n = parent->get_child_count();
        for (int i = 0; i < n; ++i) {
            auto* s = Object::cast_to<ForgeDockingContainerControl>(parent->get_child(i));
            if (!s || s == this) continue;
            if (s->get_dock_side() == side.to_lower()) return s;
        }
        return nullptr;
    }

    // -----------------------------------------------------------------------
    // Members
    // -----------------------------------------------------------------------
    PopupMenu*                              popup_         = nullptr;
    PanelContainer*                         dialog_        = nullptr;
    Control*                                overlay_       = nullptr;
    std::map<std::string, Button*>          dock_btns_;
    std::vector<FloatEntry>                 float_entries_;
    String                                  dock_side_     = "center";
    float                                   fixed_width_   = -1.0f;
    float                                   base_fixed_width_ = -1.0f;
    float                                   fixed_height_  = -1.0f;
    float                                   height_percent_= -1.0f;
    bool                                    flex_          = false;
    bool                                    collapsed_     = false;
};

// ---------------------------------------------------------------------------
// ForgeDockingHostControl
// ---------------------------------------------------------------------------

class ForgeDockingHostControl : public Container {
    GDCLASS(ForgeDockingHostControl, Container);

    struct DragState { bool active = false; float origin = 0.f; float initial = 0.f; };

    static constexpr int   MAX_H     = 4;     // horizontal gap handles (up to 4 column gaps)
    static constexpr int   MAX_V     = 4;     // vertical split handles (farleft,left,right,farright)
    static constexpr float MIN_COL_W = 60.f;

protected:
    static void _bind_methods() {
        ClassDB::bind_method(D_METHOD("set_gap", "value"), &ForgeDockingHostControl::set_gap);
        ClassDB::bind_method(D_METHOD("get_gap"),          &ForgeDockingHostControl::get_gap);
        ClassDB::bind_method(D_METHOD("_on_h_handle_input", "event", "idx"),
                             &ForgeDockingHostControl::_on_h_handle_input);
        ClassDB::bind_method(D_METHOD("_on_v_handle_input", "event", "idx"),
                             &ForgeDockingHostControl::_on_v_handle_input);
        ClassDB::bind_method(D_METHOD("_on_overlay_input", "event"),
                             &ForgeDockingHostControl::_on_overlay_input);
        ClassDB::bind_method(D_METHOD("_on_h_handle_entered", "idx"),
                             &ForgeDockingHostControl::_on_h_handle_entered);
        ClassDB::bind_method(D_METHOD("_on_h_handle_exited", "idx"),
                             &ForgeDockingHostControl::_on_h_handle_exited);
        ClassDB::bind_method(D_METHOD("_on_v_handle_entered", "idx"),
                             &ForgeDockingHostControl::_on_v_handle_entered);
        ClassDB::bind_method(D_METHOD("_on_v_handle_exited", "idx"),
                             &ForgeDockingHostControl::_on_v_handle_exited);
    }

public:
    void   set_gap(double v) { gap_ = (float)(v < 0.0 ? 0.0 : v); queue_sort(); }
    double get_gap()   const { return gap_; }

    void _ready() override {
        // Prevent visual overdraw between neighbouring dock columns on tight widths.
        set_clip_contents(true);
        if (Window* win = get_window()) {
            window_base_min_size_ = win->get_min_size();
            window_base_min_captured_ = true;
        }
        _ensure_auto_dock_containers();
        queue_sort();
    }

    void _notification(int what) {
        if (what == NOTIFICATION_SORT_CHILDREN) arrange_children();
    }

    // --- Horizontal resize handle input ---
    void _on_h_handle_input(Ref<InputEvent> event, int idx) {
        if (idx < 0 || idx >= MAX_H) return;
        Ref<InputEventMouseButton> mb = event;
        if (mb.is_valid() && mb->get_button_index() == MouseButton::MOUSE_BUTTON_LEFT
                          && !mb->is_pressed() && overlay_h_idx_ == idx) {
            // Godot sends release to the control that received the press — end drag here.
            _end_drag();
            return;
        }
        if (mb.is_valid() && mb->get_button_index() == MouseButton::MOUSE_BUTTON_LEFT
                          && mb->is_pressed()) {
            drag_target_dock_ = h_left_[idx];  // save reference — h_left_ is cleared each layout
            float init_w = 0.f;
            if (drag_target_dock_) {
                double fw = drag_target_dock_->get_fixed_width();
                init_w = (fw > 0.0) ? (float)fw : (float)drag_target_dock_->get_size().x;
            }
            h_drag_[idx] = { true, mb->get_global_position().x, init_w };
            if (h_handles_[idx]) h_handles_[idx]->set_color(COL_DRAG());
            overlay_h_idx_ = idx;
            overlay_v_idx_ = -1;
            _show_overlay(Control::CURSOR_HSIZE);
        }
    }

    // --- Vertical resize handle input ---
    void _on_v_handle_input(Ref<InputEvent> event, int idx) {
        if (idx < 0 || idx >= MAX_V) return;
        Ref<InputEventMouseButton> mb = event;
        if (mb.is_valid() && mb->get_button_index() == MouseButton::MOUSE_BUTTON_LEFT
                          && !mb->is_pressed() && overlay_v_idx_ == idx) {
            // Godot sends release to the control that received the press — end drag here.
            _end_drag();
            return;
        }
        if (mb.is_valid() && mb->get_button_index() == MouseButton::MOUSE_BUTTON_LEFT
                          && mb->is_pressed()) {
            drag_target_dock_ = v_bot_[idx];  // save reference — v_bot_ is cleared each layout
            float init_pct = 50.f;
            if (drag_target_dock_) {
                float hp = (float)drag_target_dock_->get_height_percent();
                if (hp > 0.f) init_pct = 100.f - hp;
            }
            v_drag_[idx] = { true, mb->get_global_position().y, init_pct };
            if (v_handles_[idx]) v_handles_[idx]->set_color(COL_DRAG());
            overlay_h_idx_ = -1;
            overlay_v_idx_ = idx;
            _show_overlay(Control::CURSOR_VSIZE);
        }
    }

    // Hover colours (not constexpr — Color has no constexpr constructor in godot-cpp)
    static Color COL_NORMAL() { return Color(0.45f, 0.45f, 0.55f, 0.40f); }
    static Color COL_HOVER()  { return Color(0.30f, 0.55f, 0.90f, 0.35f); }
    static Color COL_DRAG()   { return Color(0.30f, 0.55f, 0.90f, 0.55f); }

    void _on_h_handle_entered(int idx) {
        if (idx < 0 || idx >= MAX_H || !h_handles_[idx]) return;
        if (!h_drag_[idx].active) h_handles_[idx]->set_color(COL_HOVER());
    }
    void _on_h_handle_exited(int idx) {
        if (idx < 0 || idx >= MAX_H || !h_handles_[idx]) return;
        if (!h_drag_[idx].active) h_handles_[idx]->set_color(COL_NORMAL());
    }
    void _on_v_handle_entered(int idx) {
        if (idx < 0 || idx >= MAX_V || !v_handles_[idx]) return;
        if (!v_drag_[idx].active) v_handles_[idx]->set_color(COL_HOVER());
    }
    void _on_v_handle_exited(int idx) {
        if (idx < 0 || idx >= MAX_V || !v_handles_[idx]) return;
        if (!v_drag_[idx].active) v_handles_[idx]->set_color(COL_NORMAL());
    }

    void _show_overlay(Control::CursorShape cursor) {
        ColorRect* ov = _ensure_overlay();
        ov->set_default_cursor_shape(cursor);
        ov->set_position(Vector2(0.f, 0.f));
        ov->set_size(get_size());
        ov->set_visible(true);
    }

    void _end_drag() {
        if (overlay_h_idx_ >= 0) {
            if (h_handles_[overlay_h_idx_])
                h_handles_[overlay_h_idx_]->set_color(COL_NORMAL());
            h_drag_[overlay_h_idx_].active = false;
        }
        if (overlay_v_idx_ >= 0) {
            if (v_handles_[overlay_v_idx_])
                v_handles_[overlay_v_idx_]->set_color(COL_NORMAL());
            v_drag_[overlay_v_idx_].active = false;
        }
        overlay_h_idx_    = -1;
        overlay_v_idx_    = -1;
        drag_target_dock_ = nullptr;
        if (drag_overlay_) drag_overlay_->set_visible(false);
    }

    // --- Overlay captures all mouse events during drag ---
    void _on_overlay_input(Ref<InputEvent> event) {
        Ref<InputEventMouseButton> mb = event;
        if (mb.is_valid() && mb->get_button_index() == MouseButton::MOUSE_BUTTON_LEFT) {
            // End drag only on release. Press is expected when drag starts.
            if (!mb->is_pressed()) _end_drag();
            return;
        }
        Ref<InputEventMouseMotion> mm = event;
        if (mm.is_valid()) {
            // Fallback: end drag if button no longer held.
            if (!Input::get_singleton()->is_mouse_button_pressed(MouseButton::MOUSE_BUTTON_LEFT)) {
                _end_drag();
                return;
            }
            if (overlay_h_idx_ >= 0 && drag_target_dock_) {
                int idx = overlay_h_idx_;
                if (h_drag_[idx].active) {
                    float delta = mm->get_global_position().x - h_drag_[idx].origin;
                    float min_w = resolve_dock_min_width(drag_target_dock_, MIN_COL_W);
                    if (h_pair_[idx]) min_w = maxf(min_w, resolve_dock_min_width(h_pair_[idx], MIN_COL_W));
                    const float new_w = maxf(min_w, h_drag_[idx].initial + delta * h_delta_sign_[idx]);
                    drag_target_dock_->set_fixed_width((double)new_w);
                    if (h_pair_[idx]) h_pair_[idx]->set_fixed_width((double)new_w);
                    queue_sort();
                }
            }
            if (overlay_v_idx_ >= 0 && drag_target_dock_) {
                int idx = overlay_v_idx_;
                if (v_drag_[idx].active) {
                    float total_h = get_size().y;
                    if (total_h > 0.f) {
                        float delta = mm->get_global_position().y - v_drag_[idx].origin;
                        const float gap_px = Math::floor(maxf(0.f, gap_));
                        float top_min = resolve_dock_min_height(v_top_[idx], 40.f);
                        float bot_min = resolve_dock_min_height(drag_target_dock_, 40.f);
                        const float top_max = maxf(top_min, total_h - gap_px - bot_min);
                        const float initial_top_h = (v_drag_[idx].initial / 100.f) * total_h;
                        const float new_top_h = clampf(initial_top_h + delta, top_min, top_max);
                        const float new_bot_h = maxf(bot_min, total_h - gap_px - new_top_h);
                        drag_target_dock_->set_height_percent((double)((new_bot_h / total_h) * 100.f));
                        queue_sort();
                    }
                }
            }
        }
    }

private:
    static float clampf(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }
    static float maxf(float a, float b) { return a > b ? a : b; }
    static float minf(float a, float b) { return a < b ? a : b; }

    static float resolve_column_width(ForgeDockingContainerControl* top,
                                      ForgeDockingContainerControl* bot, float fallback) {
        float w = fallback;
        if (top && top->get_fixed_width() > 0.0) w = (float)top->get_fixed_width();
        if (bot && bot->get_fixed_width() > 0.0) w = (float)bot->get_fixed_width();
        return maxf(1.f, w);
    }

    static float resolve_column_min_width(ForgeDockingContainerControl* top,
                                          ForgeDockingContainerControl* bot, float fallback) {
        float w = fallback;
        if (top) {
            w = maxf(w, top->get_custom_minimum_size().x);
        }
        if (bot) {
            w = maxf(w, bot->get_custom_minimum_size().x);
        }
        return maxf(1.f, w);
    }

    static float resolve_center_min_width(ForgeDockingContainerControl* center, float fallback) {
        float w = fallback;
        if (!center) return maxf(1.f, w);
        w = maxf(w, center->get_custom_minimum_size().x);
        const int tabs = center->get_tab_count();
        for (int i = 0; i < tabs; ++i) {
            Control* tab = center->get_tab_control(i);
            if (!tab) continue;
            w = maxf(w, tab->get_custom_minimum_size().x);
        }
        return maxf(1.f, w);
    }

    static float resolve_dock_min_width(ForgeDockingContainerControl* dock, float fallback) {
        float w = fallback;
        if (!dock) return maxf(1.f, w);
        w = maxf(w, dock->get_custom_minimum_size().x);
        return maxf(1.f, w);
    }

    static float resolve_dock_min_height(ForgeDockingContainerControl* dock, float fallback) {
        float h = fallback;
        if (!dock) return maxf(1.f, h);
        h = maxf(h, dock->get_custom_minimum_size().y);
        return maxf(1.f, h);
    }

    static float resolve_bottom_height(ForgeDockingContainerControl* bot, float total_h) {
        if (!bot) return 0.f;
        const float fixed_h = (float)bot->get_fixed_height();
        if (fixed_h > 0.f) return clampf(fixed_h, 0.f, total_h);
        const float pct = (float)bot->get_height_percent();
        if (pct > 0.f) return clampf(total_h * (pct / 100.f), 0.f, total_h);
        return total_h * 0.5f;
    }

    ColorRect* _ensure_overlay() {
        if (drag_overlay_) return drag_overlay_;
        drag_overlay_ = memnew(ColorRect);
        drag_overlay_->set_color(Color(0.f, 0.f, 0.f, 0.f));  // fully transparent
        drag_overlay_->set_mouse_filter(Control::MOUSE_FILTER_STOP);
        drag_overlay_->set_z_index(4095);
        drag_overlay_->set_visible(false);
        drag_overlay_->connect("gui_input",
            callable_mp(this, &ForgeDockingHostControl::_on_overlay_input));
        add_child(drag_overlay_);
        return drag_overlay_;
    }

    // Layout top+bottom into rect; returns gap top-y (for v-handle placement), -1 if no split
    float layout_column(ForgeDockingContainerControl* top, ForgeDockingContainerControl* bot,
                        const Rect2& rect, float gap_px) {
        if (top && bot) {
            const float total_h = maxf(0.f, rect.size.y);
            const float bot_max = maxf(0.f, total_h - gap_px);
            const float bot_h   = Math::floor(clampf(resolve_bottom_height(bot, total_h), 0.f, bot_max));
            const float bot_y   = rect.position.y + total_h - bot_h;
            const float top_h   = maxf(0.f, bot_y - rect.position.y - gap_px);
            fit_child_in_rect(top, Rect2(rect.position, Vector2(rect.size.x, top_h)));
            fit_child_in_rect(bot, Rect2(Vector2(rect.position.x, bot_y), Vector2(rect.size.x, bot_h)));
            return bot_y - gap_px;  // gap top-y (where v-handle goes)
        }
        if (top) { fit_child_in_rect(top, rect); }
        if (bot) { fit_child_in_rect(bot, rect); }
        return -1.f;
    }


    ColorRect* _ensure_h_handle(int idx) {
        if (h_handles_[idx]) return h_handles_[idx];
        ColorRect* cr = memnew(ColorRect);
        cr->set_color(COL_NORMAL());
        cr->set_mouse_filter(Control::MOUSE_FILTER_STOP);
        cr->set_default_cursor_shape(Control::CURSOR_HSIZE);
        cr->set_z_index(900);
        cr->connect("gui_input",
            callable_mp(this, &ForgeDockingHostControl::_on_h_handle_input).bind(Variant(idx)));
        cr->connect("mouse_entered",
            callable_mp(this, &ForgeDockingHostControl::_on_h_handle_entered).bind(Variant(idx)));
        cr->connect("mouse_exited",
            callable_mp(this, &ForgeDockingHostControl::_on_h_handle_exited).bind(Variant(idx)));
        add_child(cr);
        h_handles_[idx] = cr;
        return cr;
    }

    ColorRect* _ensure_v_handle(int idx) {
        if (v_handles_[idx]) return v_handles_[idx];
        ColorRect* cr = memnew(ColorRect);
        cr->set_color(COL_NORMAL());
        cr->set_mouse_filter(Control::MOUSE_FILTER_STOP);
        cr->set_default_cursor_shape(Control::CURSOR_VSIZE);
        cr->set_z_index(900);
        cr->connect("gui_input",
            callable_mp(this, &ForgeDockingHostControl::_on_v_handle_input).bind(Variant(idx)));
        cr->connect("mouse_entered",
            callable_mp(this, &ForgeDockingHostControl::_on_v_handle_entered).bind(Variant(idx)));
        cr->connect("mouse_exited",
            callable_mp(this, &ForgeDockingHostControl::_on_v_handle_exited).bind(Variant(idx)));
        add_child(cr);
        v_handles_[idx] = cr;
        return cr;
    }

    void arrange_children() {
        // 5 columns: 0=farleft, 1=left, 2=center, 3=right, 4=farright
        // Bottom rows:          0=farleftbottom, 1=leftbottom, -, 3=rightbottom, 4=farrightbottom
        ForgeDockingContainerControl* col_top[5] = {};
        ForgeDockingContainerControl* col_bot[5] = {};
        std::vector<ForgeDockingContainerControl*> centers;

        const int n = get_child_count();
        for (int i = 0; i < n; ++i) {
            auto* dock = Object::cast_to<ForgeDockingContainerControl>(get_child(i));
            if (!dock) continue;
            // Drag-to-rearrange can bypass our explicit move path. Ensure empty docks
            // do not keep occupying layout space.
            if (dock->get_tab_count() == 0) {
                dock->set_visible(false);
                continue;
            }
            if (!dock->is_visible()) continue;
            const String side = dock->get_dock_side().to_lower();
            if      (side == "farleft")        col_top[0] = dock;
            else if (side == "farleftbottom")  col_bot[0] = dock;
            else if (side == "left")           col_top[1] = dock;
            else if (side == "leftbottom")     col_bot[1] = dock;
            else if (side == "right")          col_top[3] = dock;
            else if (side == "rightbottom")    col_bot[3] = dock;
            else if (side == "farright")       col_top[4] = dock;
            else if (side == "farrightbottom") col_bot[4] = dock;
            else                               centers.push_back(dock);
        }

        // Center uses first item for sizing; rest stacked invisibly
        bool has[5];
        for (int i = 0; i < 5; ++i) has[i] = (col_top[i] || col_bot[i]);
        if (!centers.empty()) has[2] = true;

        const float total_w = get_size().x;
        const float total_h = get_size().y;
        const float gap_px  = Math::floor(maxf(0.f, gap_));
        const float h_thick = maxf(gap_px, 4.f);  // handle click target width

        // Fixed widths for side columns; center is whatever remains
        float cw[5] = {};
        float cmin[5] = {};
        cw[0] = has[0] ? resolve_column_width(col_top[0], col_bot[0], 220.f) : 0.f;
        cw[1] = has[1] ? resolve_column_width(col_top[1], col_bot[1], 240.f) : 0.f;
        cw[3] = has[3] ? resolve_column_width(col_top[3], col_bot[3], 240.f) : 0.f;
        cw[4] = has[4] ? resolve_column_width(col_top[4], col_bot[4], 220.f) : 0.f;
        cmin[0] = has[0] ? resolve_column_min_width(col_top[0], col_bot[0], MIN_COL_W) : 0.f;
        cmin[1] = has[1] ? resolve_column_min_width(col_top[1], col_bot[1], MIN_COL_W) : 0.f;
        cmin[3] = has[3] ? resolve_column_min_width(col_top[3], col_bot[3], MIN_COL_W) : 0.f;
        cmin[4] = has[4] ? resolve_column_min_width(col_top[4], col_bot[4], MIN_COL_W) : 0.f;

        int gap_count = 0;
        int prev_vis  = -1;
        for (int i = 0; i < 5; ++i) {
            if (has[i]) { if (prev_vis >= 0) ++gap_count; prev_vis = i; }
        }
        const float gaps_w     = (float)gap_count * gap_px;
        const float avail_cols = maxf(0.f, total_w - gaps_w);
        float center_min = 0.f;
        if (has[2]) {
            center_min = resolve_center_min_width(centers.empty() ? nullptr : centers.front(), MIN_COL_W);
        }

        // Dynamic docking host minimum width: once all columns are at min width,
        // prevent additional window shrink that would force overlap.
        const float layout_min_w = gaps_w + cmin[0] + cmin[1] + center_min + cmin[3] + cmin[4];
        set_custom_minimum_size(Vector2(layout_min_w, 0.f));
        if (Window* win = get_window()) {
            if (!window_base_min_captured_) {
                window_base_min_size_ = win->get_min_size();
                window_base_min_captured_ = true;
            }
            // Include non-docking horizontal space (other UI + paddings) so the
            // window minimum reflects the complete layout.
            const float non_host_w = maxf(0.f, (float)win->get_size().x - total_w);
            const int dock_required_w = (int)Math::ceil((double)(layout_min_w + non_host_w));
            const int min_w = dock_required_w > window_base_min_size_.x ? dock_required_w : window_base_min_size_.x;
            const Vector2i target_min(min_w, window_base_min_size_.y);
            if (win->get_min_size() != target_min) {
                win->set_min_size(target_min);
            }
        }

        // Center shrinks first (down to center_min), then side columns shrink in UX order.
        float side_sum = cw[0] + cw[1] + cw[3] + cw[4];
        const float max_side_sum = has[2] ? maxf(0.f, avail_cols - center_min) : avail_cols;
        float overflow = maxf(0.f, side_sum - max_side_sum);
        if (overflow > 0.f) {
            static const int SHRINK_ORDER[4] = { 3, 4, 1, 0 };
            for (int si = 0; si < 4 && overflow > 0.f; ++si) {
                const int ci = SHRINK_ORDER[si];
                if (!has[ci]) continue;
                const float can_shrink = maxf(0.f, cw[ci] - cmin[ci]);
                const float take = minf(can_shrink, overflow);
                cw[ci] -= take;
                overflow -= take;
            }
        }

        side_sum = cw[0] + cw[1] + cw[3] + cw[4];
        cw[2] = maxf(0.f, avail_cols - side_sum);

        // Hide all handles before re-placing
        for (int i = 0; i < MAX_H; ++i) {
            h_left_[i] = nullptr;
            h_pair_[i] = nullptr;
            h_delta_sign_[i] = 1.f;
            if (h_handles_[i]) {
                if (!h_drag_[i].active) h_handles_[i]->set_color(COL_NORMAL());
                h_handles_[i]->set_visible(false);
            }
        }
        for (int i = 0; i < MAX_V; ++i) {
            v_bot_[i] = nullptr;
            v_top_[i] = nullptr;
            if (v_handles_[i]) {
                if (!v_drag_[i].active) v_handles_[i]->set_color(COL_NORMAL());
                v_handles_[i]->set_visible(false);
            }
        }

        // V-handle index by column: 0=farleft, 1=left, -=center, 2=right, 3=farright
        static const int V_IDX[5] = { 0, 1, -1, 2, 3 };

        float x     = 0.f;
        int   h_idx = 0;
        prev_vis    = -1;

        for (int ci = 0; ci < 5; ++ci) {
            if (!has[ci]) continue;

            // Horizontal handle between columns — always placed, min 4px, overlaid if no gap
            if (prev_vis >= 0) {
                const float hw  = maxf(gap_px, 4.f);
                const float hx  = (gap_px > 0.f) ? x : (x - hw * 0.5f);
                ColorRect* hh   = _ensure_h_handle(h_idx);
                fit_child_in_rect(hh, Rect2(hx, 0.f, hw, total_h));
                hh->set_visible(true);
                // Usually resize the left neighbour.
                // For handles with center on the left (center|right) or farRight on the
                // right (right|farRight), resize the current/right column instead.
                // In that mode the drag happens on the left edge of the resized column,
                // so the horizontal delta must be inverted.
                if (prev_vis == 2 || ci == 4) {
                    h_left_[h_idx] = col_top[ci] ? col_top[ci] : col_bot[ci];
                    h_pair_[h_idx] = (col_top[ci] && col_bot[ci])
                        ? (h_left_[h_idx] == col_top[ci] ? col_bot[ci] : col_top[ci])
                        : nullptr;
                    h_delta_sign_[h_idx] = -1.f;
                } else {
                    h_left_[h_idx] = col_top[prev_vis] ? col_top[prev_vis] : col_bot[prev_vis];
                    h_pair_[h_idx] = (col_top[prev_vis] && col_bot[prev_vis])
                        ? (h_left_[h_idx] == col_top[prev_vis] ? col_bot[prev_vis] : col_top[prev_vis])
                        : nullptr;
                    h_delta_sign_[h_idx] = 1.f;
                }
                ++h_idx;
                x += gap_px;
            }

            Rect2 col_rect(x, 0.f, cw[ci], total_h);

            if (ci == 2) {
                // Center: all extras stacked at zero size
                if (!centers.empty()) {
                    fit_child_in_rect(centers.front(), col_rect);
                    for (std::size_t k = 1; k < centers.size(); ++k)
                        fit_child_in_rect(centers[k], Rect2(col_rect.position, Vector2(0.f, 0.f)));
                }
            } else {
                float gap_y = layout_column(col_top[ci], col_bot[ci], col_rect, gap_px);
                // Vertical handle for split columns — always placed, min 4px, overlaid if no gap
                if (col_top[ci] && col_bot[ci] && gap_y >= 0.f) {
                    int vi = V_IDX[ci];
                    if (vi >= 0) {
                        const float vh_h = maxf(gap_px, 4.f);
                        const float vh_y = (gap_px > 0.f) ? gap_y : (gap_y - vh_h * 0.5f);
                        ColorRect* vh = _ensure_v_handle(vi);
                        fit_child_in_rect(vh, Rect2(x, vh_y, cw[ci], vh_h));
                        vh->set_visible(true);
                        v_top_[vi] = col_top[ci];
                        v_bot_[vi] = col_bot[ci];
                    }
                }
            }

            x += cw[ci];
            prev_vis = ci;
        }
    }

    void _ensure_auto_dock_containers() {
        static constexpr const char* SIDES[9] = {
            "farleft", "farleftbottom",
            "left",    "leftbottom",
            "center",
            "right",   "rightbottom",
            "farright","farrightbottom"
        };

        std::vector<ForgeDockingContainerControl*> existing;
        int n = get_child_count();
        for (int i = 0; i < n; ++i) {
            auto* d = Object::cast_to<ForgeDockingContainerControl>(get_child(i));
            if (d) existing.push_back(d);
        }

        if (existing.empty()) return;

        int drag_group = 1;
        for (auto* d : existing) {
            int rg = d->get_tabs_rearrange_group();
            if (rg > 0) { drag_group = rg; break; }
        }

        for (const char* side : SIDES) {
            bool found = false;
            for (auto* d : existing) {
                if (d->get_dock_side() == String(side)) { found = true; break; }
            }
            if (found) continue;

            auto* auto_dock = memnew(ForgeDockingContainerControl);
            auto_dock->set_name(String("Auto_") + side);
            auto_dock->set_dock_side(String(side));
            auto_dock->set_drag_to_rearrange_enabled(true);
            auto_dock->set_tabs_rearrange_group(drag_group);
            auto_dock->set_fixed_width(220.0);
            auto_dock->set_visible(false);
            add_child(auto_dock);
            existing.push_back(auto_dock);
        }
    }

    float gap_ = 0.f;

    ColorRect*                    h_handles_[MAX_H] = {};
    ColorRect*                    v_handles_[MAX_V] = {};
    ForgeDockingContainerControl* h_left_[MAX_H]    = {};  // left neighbour for each h-handle
    ForgeDockingContainerControl* h_pair_[MAX_H]    = {};  // second dock in same resized column (if split)
    float                         h_delta_sign_[MAX_H] = {1.f, 1.f, 1.f, 1.f}; // width delta sign per h-handle
    ForgeDockingContainerControl* v_top_[MAX_V]     = {};  // top container for each v-handle
    ForgeDockingContainerControl* v_bot_[MAX_V]     = {};  // bottom container for each v-handle
    DragState                     h_drag_[MAX_H]    = {};
    DragState                     v_drag_[MAX_V]    = {};
    Vector2i                      window_base_min_size_ = Vector2i(0, 0);
    bool                          window_base_min_captured_ = false;
    // Drag capture overlay — shown during resize drag to capture all mouse events.
    ColorRect*                    drag_overlay_     = nullptr;
    int                           overlay_h_idx_    = -1;  // active h-drag index, -1 if none
    int                           overlay_v_idx_    = -1;  // active v-drag index, -1 if none
    ForgeDockingContainerControl* drag_target_dock_ = nullptr; // saved at drag start
};

// ---------------------------------------------------------------------------
// ForgeWindowDragControl — transparent drag area that moves the OS window
// ---------------------------------------------------------------------------

class ForgeWindowDragControl : public Control {
    GDCLASS(ForgeWindowDragControl, Control);
protected:
    static void _bind_methods() {}
public:
    void _gui_input(const Ref<InputEvent>& p_event) override {
        Ref<InputEventMouseButton> mb = p_event;
        if (mb.is_valid() &&
            mb->get_button_index() == MouseButton::MOUSE_BUTTON_LEFT &&
            mb->is_pressed()) {
            Window* win = get_window();
            if (!win) return;
            if (mb->is_double_click()) {
                auto* ds = DisplayServer::get_singleton();
                bool maximized = ds->window_get_mode(win->get_window_id())
                                 == DisplayServer::WINDOW_MODE_MAXIMIZED;
                ds->window_set_mode(
                    maximized ? DisplayServer::WINDOW_MODE_WINDOWED
                              : DisplayServer::WINDOW_MODE_MAXIMIZED,
                    win->get_window_id());
            } else {
                DisplayServer::get_singleton()->window_start_drag(win->get_window_id());
            }
        }
    }
};

// ---------------------------------------------------------------------------
// ForgeSpeechRecognizerControl
// Native speech hook + text post-processing pipeline scaffold.
// ---------------------------------------------------------------------------

class ForgeSpeechRecognizerControl : public Control {
    GDCLASS(ForgeSpeechRecognizerControl, Control);

protected:
    static void _bind_methods() {
        ClassDB::bind_method(D_METHOD("listen"), &ForgeSpeechRecognizerControl::listen);
        ClassDB::bind_method(D_METHOD("stop"), &ForgeSpeechRecognizerControl::stop);
        ClassDB::bind_method(D_METHOD("submitText", "text"), &ForgeSpeechRecognizerControl::submit_text);
        ClassDB::bind_method(D_METHOD("_bridgeRawResult", "text"), &ForgeSpeechRecognizerControl::bridge_raw_result);
        ClassDB::bind_method(D_METHOD("_bridgeResult", "text"), &ForgeSpeechRecognizerControl::bridge_result);
        ClassDB::bind_method(D_METHOD("_bridgeError", "message"), &ForgeSpeechRecognizerControl::bridge_error);

        ClassDB::bind_method(D_METHOD("set_language", "value"), &ForgeSpeechRecognizerControl::set_language);
        ClassDB::bind_method(D_METHOD("get_language"), &ForgeSpeechRecognizerControl::get_language);
        ClassDB::bind_method(D_METHOD("set_mode", "value"), &ForgeSpeechRecognizerControl::set_mode);
        ClassDB::bind_method(D_METHOD("get_mode"), &ForgeSpeechRecognizerControl::get_mode);
        ClassDB::bind_method(D_METHOD("set_filters", "value"), &ForgeSpeechRecognizerControl::set_filters);
        ClassDB::bind_method(D_METHOD("get_filters"), &ForgeSpeechRecognizerControl::get_filters);
        ClassDB::bind_method(D_METHOD("set_suffix", "value"), &ForgeSpeechRecognizerControl::set_suffix);
        ClassDB::bind_method(D_METHOD("get_suffix"), &ForgeSpeechRecognizerControl::get_suffix);
        ClassDB::bind_method(D_METHOD("setMode", "value"), &ForgeSpeechRecognizerControl::set_mode);
        ClassDB::bind_method(D_METHOD("getMode"), &ForgeSpeechRecognizerControl::get_mode);
        ClassDB::bind_method(D_METHOD("setFilters", "value"), &ForgeSpeechRecognizerControl::set_filters);
        ClassDB::bind_method(D_METHOD("getFilters"), &ForgeSpeechRecognizerControl::get_filters);
        ClassDB::bind_method(D_METHOD("setSuffix", "value"), &ForgeSpeechRecognizerControl::set_suffix);
        ClassDB::bind_method(D_METHOD("getSuffix"), &ForgeSpeechRecognizerControl::get_suffix);
        ClassDB::bind_method(D_METHOD("setLanguage", "value"), &ForgeSpeechRecognizerControl::set_language);
        ClassDB::bind_method(D_METHOD("getLanguage"), &ForgeSpeechRecognizerControl::get_language);

        ADD_PROPERTY(PropertyInfo(Variant::STRING, "language"), "set_language", "get_language");
        ADD_PROPERTY(PropertyInfo(Variant::STRING, "mode"), "set_mode", "get_mode");
        ADD_PROPERTY(PropertyInfo(Variant::STRING, "filters"), "set_filters", "get_filters");
        ADD_PROPERTY(PropertyInfo(Variant::STRING, "suffix"), "set_suffix", "get_suffix");

        ADD_SIGNAL(MethodInfo("started"));
        ADD_SIGNAL(MethodInfo("stopped"));
        ADD_SIGNAL(MethodInfo("rawResult", PropertyInfo(Variant::STRING, "text")));
        ADD_SIGNAL(MethodInfo("partialResult", PropertyInfo(Variant::STRING, "text")));
        ADD_SIGNAL(MethodInfo("result", PropertyInfo(Variant::STRING, "text")));
        ADD_SIGNAL(MethodInfo("error", PropertyInfo(Variant::STRING, "message")));
    }

public:
    void _ready() override {
        // Non-visual helper control.
        set_custom_minimum_size(Vector2(0, 0));
        set_visible(false);
        set_process(true);
    }

    void _process(double) override {
        poll_bridge_events();
    }

    void listen() {
        emit_signal("started");

        OS* os = OS::get_singleton();
        Engine* engine = Engine::get_singleton();
        if (os == nullptr) {
            emit_signal("error", String("SpeechRecognizer backend unavailable."));
            return;
        }

        const bool is_android = os->has_feature("android");
        if (!is_android) {
            emit_signal("error", String("SpeechRecognizer native capture is Android-only. Use keyboard mic or submitText()."));
            return;
        }

        // Request microphone permission (if already granted this is effectively a no-op).
        const bool permission_ok = os->request_permission("android.permission.RECORD_AUDIO");
        if (!permission_ok) {
            if (engine != nullptr && engine->has_singleton("ForgeSpeechBridge")) {
                Object* bridge = engine->get_singleton("ForgeSpeechBridge");
                if (bridge != nullptr) {
                    bridge->call("showToast", String("Microphone permission denied."));
                }
            }
            emit_signal("error", String("Microphone permission denied."));
            return;
        }

        // Preferred path: fixed Android host singleton bridge.
        // Security: method names and singleton name are hardcoded in C++, not script-provided.
        if (engine != nullptr && engine->has_singleton("ForgeSpeechBridge")) {
            Object* bridge = engine->get_singleton("ForgeSpeechBridge");
            if (bridge != nullptr) {
                const Variant ret = bridge->call("startListening", static_cast<int64_t>(get_instance_id()), language_);
                bool started = false;
                if (ret.get_type() == Variant::BOOL) {
                    started = static_cast<bool>(ret);
                } else if (ret.get_type() == Variant::INT) {
                    started = static_cast<int64_t>(ret) != 0;
                }
                if (started) {
                    return;
                }
                bridge->call("showToast", String("Could not start speech recognition."));
            }
            emit_signal("error", String("ForgeSpeechBridge startListening failed."));
            return;
        }

        emit_signal("error", String("ForgeSpeechBridge not found."));
    }

    void stop() {
        Engine* engine = Engine::get_singleton();
        if (engine != nullptr && engine->has_singleton("ForgeSpeechBridge")) {
            Object* bridge = engine->get_singleton("ForgeSpeechBridge");
            if (bridge != nullptr) {
                bridge->call("stopListening", static_cast<int64_t>(get_instance_id()));
            }
        }
        emit_signal("stopped");
    }

    void submit_text(const String& text) {
        emit_signal("rawResult", text);
        const String processed = apply_mode(text);
        emit_signal("result", processed);
    }

    // Bridge entry points for Android host singleton callbacks.
    void bridge_raw_result(const String& text) {
        emit_signal("rawResult", text);
        emit_signal("partialResult", text);
    }

    void bridge_result(const String& text) {
        emit_signal("rawResult", text);
        const String processed = apply_mode(text);
        emit_signal("result", processed);
    }

    void bridge_error(const String& message) {
        emit_signal("error", message);
    }

    void set_language(const String& value) {
        language_ = value.strip_edges();
        if (language_.is_empty()) {
            language_ = "de-DE";
        }
    }
    String get_language() const { return language_; }

    void set_mode(const String& value) {
        const String normalized = value.to_lower().strip_edges();
        if (normalized == "raw" || normalized == "clean" || normalized == "markdown") {
            mode_ = normalized;
            return;
        }
        mode_ = "clean";
    }
    String get_mode() const { return mode_; }

    void set_filters(const String& value) {
        filters_ = value.strip_edges();
    }
    String get_filters() const { return filters_; }

    void set_suffix(const String& value) {
        suffix_ = value;
    }
    String get_suffix() const { return suffix_; }

private:
    void poll_bridge_events() {
        Engine* engine = Engine::get_singleton();
        if (engine == nullptr || !engine->has_singleton("ForgeSpeechBridge")) {
            return;
        }

        Object* bridge = engine->get_singleton("ForgeSpeechBridge");
        if (bridge == nullptr) return;

        const int64_t object_id = static_cast<int64_t>(get_instance_id());
        for (int i = 0; i < 32; ++i) {
            const Variant kind_var = bridge->call("popEventKind", object_id);
            int64_t kind = 0;
            if (kind_var.get_type() == Variant::INT) {
                kind = static_cast<int64_t>(kind_var);
            } else if (kind_var.get_type() == Variant::FLOAT) {
                kind = static_cast<int64_t>(static_cast<double>(kind_var));
            } else {
                break;
            }
            if (kind <= 0) break;

            const Variant text_var = bridge->call("popEventText", object_id);
            String text;
            if (text_var.get_type() == Variant::STRING) {
                text = static_cast<String>(text_var);
            } else {
                text = String();
            }

            if (kind == 1) {
                bridge_raw_result(text);
            } else if (kind == 2) {
                bridge_result(text);
            } else if (kind == 3) {
                bridge_error(text);
            }
        }
    }

    static std::string to_lower_ascii(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return s;
    }

    static std::string trim_ascii(const std::string& s) {
        size_t start = 0;
        while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) ++start;
        if (start == s.size()) return "";
        size_t end = s.size() - 1;
        while (end > start && std::isspace(static_cast<unsigned char>(s[end]))) --end;
        return s.substr(start, (end - start) + 1);
    }

    static void replace_all_case_variants(std::string& text, const std::string& needle_raw) {
        const std::string needle = trim_ascii(needle_raw);
        if (needle.empty()) return;

        const std::string lower = to_lower_ascii(needle);
        std::string upper = lower;
        std::transform(upper.begin(), upper.end(), upper.begin(),
                       [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
        std::string title = lower;
        if (!title.empty()) {
            title[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(title[0])));
        }

        const std::string candidates[3] = { lower, upper, title };
        for (const auto& candidate : candidates) {
            size_t pos = 0;
            while (true) {
                pos = text.find(candidate, pos);
                if (pos == std::string::npos) break;
                text.replace(pos, candidate.size(), " ");
                pos += 1;
            }
        }
    }

    static std::string collapse_spaces(const std::string& in) {
        std::string out;
        out.reserve(in.size());
        bool prev_space = false;
        for (char c : in) {
            if (std::isspace(static_cast<unsigned char>(c))) {
                if (!prev_space) {
                    out.push_back(' ');
                    prev_space = true;
                }
            } else {
                out.push_back(c);
                prev_space = false;
            }
        }
        return trim_ascii(out);
    }

    static void replace_phrase_case_insensitive(std::string& text, const std::string& phrase, const std::string& replacement) {
        if (phrase.empty()) return;
        const std::string text_lower = to_lower_ascii(text);
        const std::string phrase_lower = to_lower_ascii(phrase);
        size_t search_from = 0;
        while (true) {
            const size_t pos = text_lower.find(phrase_lower, search_from);
            if (pos == std::string::npos) break;
            text.replace(pos, phrase.size(), replacement);
            search_from = pos + replacement.size();
        }
    }

    String apply_clean(const String& text) const {
        std::string out = std::string(text.utf8().get_data());
        const std::string filters = std::string(filters_.utf8().get_data());

        std::stringstream ss(filters);
        std::string item;
        while (std::getline(ss, item, ',')) {
            replace_all_case_variants(out, item);
        }

        out = collapse_spaces(out);
        return String::utf8(out.c_str());
    }

    String apply_markdown(const String& text) const {
        std::string out = std::string(apply_clean(text).utf8().get_data());

        replace_phrase_case_insensitive(out, "neuer absatz", "\n\n");
        replace_phrase_case_insensitive(out, "neue zeile", "\n");
        out = trim_ascii(out);

        if (!out.empty()) {
            const char tail = out.back();
            if (tail != '.' && tail != '!' && tail != '?') {
                out.push_back('.');
            }
        }

        return String::utf8(out.c_str());
    }

    String apply_mode(const String& text) const {
        if (mode_ == "raw") {
            return text + suffix_;
        }
        if (mode_ == "markdown") {
            return apply_markdown(text) + suffix_;
        }
        return apply_clean(text) + suffix_;
    }

    String language_ = "de-DE";
    String mode_ = "clean";
    String filters_ = "zdf,wdr,applaus,musik";
    String suffix_;
};

// ---------------------------------------------------------------------------
// ForgeBgVBoxContainer / ForgeBgHBoxContainer
// VBoxContainer / HBoxContainer with custom _draw() for bgColor support.
// StyleBoxFlat is set via set_bg_style(); it handles bg, border, radius, shadow.
// ---------------------------------------------------------------------------

class ForgeBgVBoxContainer : public VBoxContainer {
    GDCLASS(ForgeBgVBoxContainer, VBoxContainer);
    Ref<StyleBoxFlat> bg_style_;
    Color             highlight_color_;
    bool              has_highlight_ = false;

protected:
    static void _bind_methods() {
        ClassDB::bind_method(D_METHOD("set_bg_style",       "style"), &ForgeBgVBoxContainer::set_bg_style);
        ClassDB::bind_method(D_METHOD("set_highlight_color","color"), &ForgeBgVBoxContainer::set_highlight_color);
    }

public:
    void set_bg_style(Ref<StyleBoxFlat> style) {
        bg_style_ = style;
        queue_redraw();
    }
    void set_highlight_color(Color color) {
        highlight_color_ = color;
        has_highlight_ = true;
        queue_redraw();
    }
    void _draw() override {
        if (bg_style_.is_valid())
            draw_style_box(bg_style_, Rect2(Vector2(), get_size()));
        if (has_highlight_) {
            float r = bg_style_.is_valid() ? (float)bg_style_->get_corner_radius(CORNER_TOP_LEFT) : 0.0f;
            Vector2 sz = get_size();
            draw_line(Vector2(r, 0.5f), Vector2(sz.x - r, 0.5f), highlight_color_);
            draw_line(Vector2(0.5f, r), Vector2(0.5f, sz.y - r), highlight_color_);
        }
    }
};

class ForgeBgHBoxContainer : public HBoxContainer {
    GDCLASS(ForgeBgHBoxContainer, HBoxContainer);
    Ref<StyleBoxFlat> bg_style_;
    Color             highlight_color_;
    bool              has_highlight_ = false;

protected:
    static void _bind_methods() {
        ClassDB::bind_method(D_METHOD("set_bg_style",       "style"), &ForgeBgHBoxContainer::set_bg_style);
        ClassDB::bind_method(D_METHOD("set_highlight_color","color"), &ForgeBgHBoxContainer::set_highlight_color);
    }

public:
    void set_bg_style(Ref<StyleBoxFlat> style) {
        bg_style_ = style;
        queue_redraw();
    }
    void set_highlight_color(Color color) {
        highlight_color_ = color;
        has_highlight_ = true;
        queue_redraw();
    }
    void _draw() override {
        if (bg_style_.is_valid())
            draw_style_box(bg_style_, Rect2(Vector2(), get_size()));
        if (has_highlight_) {
            float r = bg_style_.is_valid() ? (float)bg_style_->get_corner_radius(CORNER_TOP_LEFT) : 0.0f;
            Vector2 sz = get_size();
            draw_line(Vector2(r, 0.5f), Vector2(sz.x - r, 0.5f), highlight_color_);
            draw_line(Vector2(0.5f, r), Vector2(0.5f, sz.y - r), highlight_color_);
        }
    }
};

// ---------------------------------------------------------------------------
// ForgeMarkdownContainer — VBoxContainer that renders a markdown string
// ---------------------------------------------------------------------------

class ForgeMarkdownContainer : public VBoxContainer {
    GDCLASS(ForgeMarkdownContainer, VBoxContainer);

    Ref<StyleBoxFlat> bg_style_;
    float             base_font_size_ = 16.f;

    // Heading font-size multipliers for levels 1–6
    static float heading_scale(int level) {
        static const float S[7] = {1.f, 2.0f, 1.6f, 1.3f, 1.1f, 1.0f, 0.9f};
        return S[(level >= 1 && level <= 6) ? level : 1];
    }

protected:
    static void _bind_methods() {
        ClassDB::bind_method(D_METHOD("set_bg_style", "style"),       &ForgeMarkdownContainer::set_bg_style);
        ClassDB::bind_method(D_METHOD("set_markdown",  "text"),       &ForgeMarkdownContainer::set_markdown);
        ClassDB::bind_method(D_METHOD("set_src",       "path"),       &ForgeMarkdownContainer::set_src);
        ClassDB::bind_method(D_METHOD("set_base_font_size", "size"),  &ForgeMarkdownContainer::set_base_font_size);
    }

public:
    void set_bg_style(Ref<StyleBoxFlat> style) { bg_style_ = style; queue_redraw(); }

    void set_base_font_size(double size) {
        base_font_size_ = (float)size;
        // No lazy rebuild here — caller must call set_markdown again if needed.
    }

    void set_markdown(const String& text) {
        _clear_children();
        if (text.is_empty()) return;
        const std::vector<forge::MarkdownBlock> blocks =
            forge::parse_markdown(text.utf8().get_data());
        for (const auto& b : blocks) _add_block(b);
    }

    void set_src(const String& path) {
        Ref<FileAccess> fa = FileAccess::open(path, FileAccess::READ);
        if (fa.is_valid())
            set_markdown(fa->get_as_text());
        else
            set_markdown(String("[i]Could not load: ") + path + "[/i]");
    }

    void _draw() override {
        if (bg_style_.is_valid())
            draw_style_box(bg_style_, Rect2(Vector2(), get_size()));
    }

private:
    void _clear_children() {
        for (int i = get_child_count() - 1; i >= 0; --i) {
            Node* c = get_child(i);
            remove_child(c);
            c->queue_free();
        }
    }

    void _add_block(const forge::MarkdownBlock& b) {
        switch (b.kind) {
            case forge::BlockKind::Heading:   _add_heading(b);    break;
            case forge::BlockKind::Paragraph: _add_paragraph(b);  break;
            case forge::BlockKind::CodeBlock: _add_code_block(b); break;
            case forge::BlockKind::ListItem:  _add_list_item(b);  break;
            case forge::BlockKind::Image:     _add_image(b);      break;
            case forge::BlockKind::HRule: {
                HSeparator* sep = memnew(HSeparator);
                sep->set_h_size_flags(Control::SIZE_EXPAND_FILL);
                add_child(sep);
                break;
            }
        }
    }

    void _add_heading(const forge::MarkdownBlock& b) {
        Label* lbl = memnew(Label);
        lbl->set_text(String::utf8(b.text.c_str()));
        lbl->add_theme_font_size_override("font_size",
            (int)(base_font_size_ * heading_scale(b.level)));
        lbl->set_h_size_flags(Control::SIZE_EXPAND_FILL);
        lbl->set_autowrap_mode(TextServer::AUTOWRAP_WORD_SMART);
        add_child(lbl);
    }

    void _add_paragraph(const forge::MarkdownBlock& b) {
        RichTextLabel* rtl = memnew(RichTextLabel);
        rtl->set_use_bbcode(true);
        rtl->set_text(String::utf8(forge::inline_to_bbcode(b.text).c_str()));
        rtl->set_fit_content(true);
        rtl->set_h_size_flags(Control::SIZE_EXPAND_FILL);
        rtl->add_theme_font_size_override("normal_font_size", (int)base_font_size_);
        add_child(rtl);
    }

    void _add_code_block(const forge::MarkdownBlock& b) {
        PanelContainer* panel = memnew(PanelContainer);
        Ref<StyleBoxFlat> style; style.instantiate();
        style->set_bg_color(Color(0.10f, 0.10f, 0.12f, 1.f));
        style->set_corner_radius_all(4);
        style->set_content_margin_all(8.f);
        panel->add_theme_stylebox_override("panel", style);
        panel->set_h_size_flags(Control::SIZE_EXPAND_FILL);

        // Escape BBCode brackets in code text, then wrap in [code]
        std::string escaped;
        for (char c : b.text) {
            if      (c == '[') escaped += "\\[";
            else if (c == ']') escaped += "\\]";
            else               escaped += c;
        }

        RichTextLabel* rtl = memnew(RichTextLabel);
        rtl->set_use_bbcode(true);
        rtl->set_text(String::utf8(("[code]" + escaped + "[/code]").c_str()));
        rtl->set_fit_content(true);
        rtl->set_h_size_flags(Control::SIZE_EXPAND_FILL);
        rtl->add_theme_font_size_override("mono_font_size",   (int)(base_font_size_ * 0.9f));
        rtl->add_theme_font_size_override("normal_font_size", (int)(base_font_size_ * 0.9f));
        panel->add_child(rtl);
        add_child(panel);
    }

    void _add_list_item(const forge::MarkdownBlock& b) {
        HBoxContainer* hbox = memnew(HBoxContainer);
        hbox->set_h_size_flags(Control::SIZE_EXPAND_FILL);

        Label* bullet = memnew(Label);
        bullet->set_text(String::chr(0x2022));
        bullet->add_theme_font_size_override("font_size", (int)base_font_size_);
        bullet->set_v_size_flags(Control::SIZE_SHRINK_BEGIN);
        hbox->add_child(bullet);

        RichTextLabel* rtl = memnew(RichTextLabel);
        rtl->set_use_bbcode(true);
        rtl->set_text(String::utf8(forge::inline_to_bbcode(b.text).c_str()));
        rtl->set_fit_content(true);
        rtl->set_h_size_flags(Control::SIZE_EXPAND_FILL);
        rtl->add_theme_font_size_override("normal_font_size", (int)base_font_size_);
        hbox->add_child(rtl);

        add_child(hbox);
    }

    void _add_image(const forge::MarkdownBlock& b) {
        TextureRect* img = memnew(TextureRect);
        img->set_h_size_flags(Control::SIZE_EXPAND_FILL);
        img->set_stretch_mode(TextureRect::STRETCH_KEEP_ASPECT_CENTERED);
        // Texture loading is handled by the UI builder (same as src: on TextureRect)
        img->set_meta("pending_src", String(b.src.c_str()));
        add_child(img);
    }
};

// ---------------------------------------------------------------------------
// ForgeNumberPickerControl
// ---------------------------------------------------------------------------

class ForgeNumberPickerControl : public LineEdit {
    GDCLASS(ForgeNumberPickerControl, LineEdit);

protected:
    static void _bind_methods() {
        ClassDB::bind_method(D_METHOD("set_numeric_config", "axis", "unit", "color", "step", "dragSensitivity", "decimals"),
                             &ForgeNumberPickerControl::set_numeric_config);
        ClassDB::bind_method(D_METHOD("set_numeric_value", "value"), &ForgeNumberPickerControl::set_numeric_value);
        ClassDB::bind_method(D_METHOD("get_numeric_value"), &ForgeNumberPickerControl::get_numeric_value);
    }

public:
    void set_numeric_config(const String& axis, const String& unit, const Color& color, double step, double drag_sensitivity, int decimals) {
        axis_ = axis.is_empty() ? String("x") : axis.to_lower();
        unit_ = unit;
        axis_color_ = color;
        step_ = static_cast<float>(MAX(0.000001, Math::abs(step)));
        drag_sensitivity_ = static_cast<float>(MAX(0.000001, Math::abs(drag_sensitivity)));
        decimals_ = CLAMP(decimals, 0, 6);

        set_horizontal_alignment(HORIZONTAL_ALIGNMENT_RIGHT);
        set_select_all_on_focus(true);
        add_theme_color_override("font_color", axis_color_);
        add_theme_color_override("caret_color", axis_color_);
        set_numeric_value(parse_numeric_from_text(get_text()));
    }

    void set_numeric_value(double value) {
        value_ = static_cast<float>(value);
        if (editing_) {
            set_text(String::num(value_));
        } else {
            set_text(String(format_preview(value_).c_str()));
        }
    }

    double get_numeric_value() {
        if (editing_) {
            value_ = static_cast<float>(parse_numeric_from_text(get_text()));
        }
        return value_;
    }

    void _notification(int what) {
        if (what == NOTIFICATION_FOCUS_ENTER) {
            editing_ = true;
            set_text(String::num(value_));
            select_all();
        } else if (what == NOTIFICATION_FOCUS_EXIT) {
            if (!dragging_) {
                value_ = static_cast<float>(parse_numeric_from_text(get_text()));
                editing_ = false;
                set_text(String(format_preview(value_).c_str()));
            }
        }
    }

    void _gui_input(const Ref<InputEvent>& event) override {
        Ref<InputEventMouseButton> mb = event;
        if (mb.is_valid() && mb->get_button_index() == MOUSE_BUTTON_LEFT) {
            if (mb->is_pressed()) {
                pressed_ = true;
                dragging_ = false;
                drag_start_pos_ = mb->get_position();
                drag_start_value_ = static_cast<float>(get_numeric_value());
                if (!editing_) {
                    accept_event();
                }
                return;
            }

            if (pressed_) {
                const bool was_dragging = dragging_;
                pressed_ = false;
                dragging_ = false;
                if (was_dragging) {
                    accept_event();
                } else if (!editing_) {
                    accept_event();
                    grab_focus();
                    select_all();
                }
            }
            return;
        }

        Ref<InputEventMouseMotion> mm = event;
        if (!mm.is_valid() || !pressed_) return;
        if (!Input::get_singleton()->is_mouse_button_pressed(MOUSE_BUTTON_LEFT)) {
            pressed_ = false;
            dragging_ = false;
            return;
        }

        const float delta_x = mm->get_position().x - drag_start_pos_.x;
        if (!dragging_ && Math::abs(delta_x) >= 2.0f) {
            if (editing_) {
                value_ = static_cast<float>(parse_numeric_from_text(get_text()));
                editing_ = false;
                release_focus();
            }
            dragging_ = true;
        }

        if (!dragging_) return;
        const float next = drag_start_value_ + ((delta_x * drag_sensitivity_) * step_);
        value_ = next;
        set_text(String(format_preview(value_).c_str()));
        emit_signal("text_changed", get_text());
        accept_event();
    }

private:
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

    std::string format_preview(float value) const {
        std::ostringstream ss;
        ss.setf(std::ios::fixed, std::ios::floatfield);
        ss << std::setprecision(decimals_) << value;
        if (unit_.is_empty()) {
            return std::string(axis_.utf8().get_data()) + " " + ss.str();
        }
        return std::string(axis_.utf8().get_data()) + " " + ss.str() + " " + std::string(unit_.utf8().get_data());
    }

    String axis_ = "x";
    String unit_;
    Color axis_color_ = Color(1.0f, 1.0f, 1.0f, 1.0f);
    int decimals_ = 3;
    float step_ = 0.01f;
    float drag_sensitivity_ = 0.02f;
    float value_ = 0.0f;
    bool editing_ = false;
    bool pressed_ = false;
    bool dragging_ = false;
    Vector2 drag_start_pos_ = Vector2();
    float drag_start_value_ = 0.0f;
};

// ---------------------------------------------------------------------------
// ForgeTimelineControl (native scaffold)
// ---------------------------------------------------------------------------

class ForgeTimelineControl : public SubViewportContainer {
    GDCLASS(ForgeTimelineControl, SubViewportContainer);

protected:
    static void _bind_methods() {
        ClassDB::bind_method(D_METHOD("setCurrentFrame", "frame"), &ForgeTimelineControl::set_current_frame);
        ClassDB::bind_method(D_METHOD("setKeyframe", "frame", "pose"), &ForgeTimelineControl::set_keyframe);
        ClassDB::bind_method(D_METHOD("setKeyframeBone", "frame", "pose", "boneName"), &ForgeTimelineControl::set_keyframe_bone);
        ClassDB::bind_method(D_METHOD("setKeyframeMove", "frame"), &ForgeTimelineControl::set_keyframe_move);
        ClassDB::bind_method(D_METHOD("setKeyframeRotate", "frame"), &ForgeTimelineControl::set_keyframe_rotate);
        ClassDB::bind_method(D_METHOD("removeKeyframe", "frame"), &ForgeTimelineControl::remove_keyframe);
        ClassDB::bind_method(D_METHOD("hasKeyframeAt", "frame"), &ForgeTimelineControl::has_keyframe_at);
        ClassDB::bind_method(D_METHOD("getPoseAt", "frame"), &ForgeTimelineControl::get_pose_at);
        ClassDB::bind_method(D_METHOD("play"), &ForgeTimelineControl::play);
        ClassDB::bind_method(D_METHOD("stop"), &ForgeTimelineControl::stop);
        ClassDB::bind_method(D_METHOD("isPlaying"), &ForgeTimelineControl::is_playing);
        ClassDB::bind_method(D_METHOD("setVisibleCharacterId", "characterId"), &ForgeTimelineControl::set_visible_character_id);
        ClassDB::bind_method(D_METHOD("getKeyframeCount"), &ForgeTimelineControl::get_keyframe_count);
        ClassDB::bind_method(D_METHOD("getKeyframeFrameAt", "index"), &ForgeTimelineControl::get_keyframe_frame_at);
        ClassDB::bind_method(D_METHOD("getKeyframeCountForCharacter", "characterId"), &ForgeTimelineControl::get_keyframe_count_for_character);
        ClassDB::bind_method(D_METHOD("getKeyframeFrameAtForCharacter", "index", "characterId"), &ForgeTimelineControl::get_keyframe_frame_at_for_character);
        ClassDB::bind_method(D_METHOD("getKeyframeBoneCountForCharacter", "frame", "characterId"), &ForgeTimelineControl::get_keyframe_bone_count_for_character);
        ClassDB::bind_method(D_METHOD("debugLogKeyframesForCharacter", "characterId"), &ForgeTimelineControl::debug_log_keyframes_for_character);
        ClassDB::bind_method(D_METHOD("clearAllKeyframes"), &ForgeTimelineControl::clear_all_keyframes);

        ClassDB::bind_method(D_METHOD("set_fps", "value"), &ForgeTimelineControl::set_fps);
        ClassDB::bind_method(D_METHOD("get_fps"), &ForgeTimelineControl::get_fps);
        ClassDB::bind_method(D_METHOD("set_total_frames", "value"), &ForgeTimelineControl::set_total_frames);
        ClassDB::bind_method(D_METHOD("get_total_frames"), &ForgeTimelineControl::get_total_frames);
        ClassDB::bind_method(D_METHOD("set_current_frame_prop", "value"), &ForgeTimelineControl::set_current_frame_prop);
        ClassDB::bind_method(D_METHOD("get_current_frame_prop"), &ForgeTimelineControl::get_current_frame_prop);

        ADD_PROPERTY(PropertyInfo(Variant::INT, "fps"), "set_fps", "get_fps");
        ADD_PROPERTY(PropertyInfo(Variant::INT, "totalFrames"), "set_total_frames", "get_total_frames");
        ADD_PROPERTY(PropertyInfo(Variant::INT, "currentFrame"), "set_current_frame_prop", "get_current_frame_prop");

        ADD_SIGNAL(MethodInfo("keyframeAdded", PropertyInfo(Variant::INT, "frame"), PropertyInfo(Variant::STRING, "boneName")));
        ADD_SIGNAL(MethodInfo("keyframeRemoved", PropertyInfo(Variant::INT, "frame")));
        ADD_SIGNAL(MethodInfo("frameChanged", PropertyInfo(Variant::INT, "frame")));
        ADD_SIGNAL(MethodInfo("playbackStarted"));
        ADD_SIGNAL(MethodInfo("playbackStopped"));
    }

public:
    void _ready() override {
        set_mouse_filter(Control::MOUSE_FILTER_STOP);
        set_focus_mode(Control::FOCUS_ALL);
        set_process(true);
        ensure_timeline_ui();
        refresh_scroll_range();
        update_frame_label();
        queue_redraw();
    }

    void _process(double delta) override {
        if (!is_playing_) return;
        const float frame_time = 1.0f / static_cast<float>(MAX(1, fps_));
        play_accumulated_ += static_cast<float>(delta);
        while (play_accumulated_ >= frame_time) {
            play_accumulated_ -= frame_time;
            int next = current_frame_ + 1;
            if (next > total_frames_) next = 0;
            set_current_frame(next);
        }
    }

    void _notification(int what) {
        if (what == NOTIFICATION_RESIZED) {
            refresh_scroll_range();
            queue_redraw();
        }
    }

    void _draw() override {
        const Vector2 size = get_size();
        if (size.x <= 1.0f || size.y <= 1.0f) return;
        const float toolbar_h = get_toolbar_height();
        const float timeline_top = toolbar_h;
        const float scrollbar_h = (scroll_bar_ != nullptr) ? MAX(12.0f, scroll_bar_->get_size().y) : 16.0f;
        const float timeline_bottom = size.y - scrollbar_h;
        if (timeline_bottom <= timeline_top + 1.0f) return;

        const float bone_name_w = 140.0f;
        const float ruler_h = 24.0f;
        const float track_h = 22.0f;
        const float timeline_left = bone_name_w;
        const float timeline_w = MAX(1.0f, size.x - timeline_left);
        const float ruler_y = timeline_top + ruler_h;
        const int frame_count = total_frames_ > 0 ? total_frames_ : 1;
        const float px_per_frame = 8.0f;
        const float content_w = static_cast<float>(frame_count + 1) * px_per_frame + 40.0f;

        Ref<Font> font = get_theme_default_font();
        const int font_size = 12;

        draw_rect(Rect2(Vector2(0.0f, timeline_top), Vector2(size.x, timeline_bottom - timeline_top)), Color(0.09f, 0.10f, 0.12f, 1.0f), true);
        draw_rect(Rect2(Vector2(timeline_left, timeline_top), Vector2(timeline_w, ruler_h)), Color(0.20f, 0.20f, 0.20f, 1.0f), true);
        draw_line(Vector2(0.0f, ruler_y), Vector2(size.x, ruler_y), Color(0.24f, 0.26f, 0.30f, 1.0f), 1.0f);
        draw_line(Vector2(timeline_left, timeline_top), Vector2(timeline_left, timeline_bottom), Color(0.24f, 0.26f, 0.30f, 1.0f), 1.0f);

        const std::vector<String> tracked_bones = get_tracked_bones_for_current_character();
        for (int row = 0; row < static_cast<int>(tracked_bones.size()); ++row) {
            const float y = ruler_y + static_cast<float>(row) * track_h;
            if ((row % 2) == 1) {
                draw_rect(Rect2(Vector2(0.0f, y), Vector2(size.x, track_h)), Color(0.11f, 0.11f, 0.11f, 1.0f), true);
            }
            draw_line(Vector2(0.0f, y + track_h), Vector2(size.x, y + track_h), Color(0.24f, 0.26f, 0.30f, 0.6f), 1.0f);
            if (font.is_valid()) {
                draw_string(font,
                            Vector2(6.0f, y + track_h * 0.66f),
                            display_bone_name(tracked_bones[row]),
                            HORIZONTAL_ALIGNMENT_LEFT,
                            static_cast<double>(bone_name_w - 10.0f),
                            font_size,
                            Color(0.82f, 0.82f, 0.82f, 1.0f));
            }
        }

        const auto char_it = keyframes_.find(current_character_key());
        for (int frame = 0; frame <= frame_count; frame += 5) {
            const float x = timeline_left + static_cast<float>(frame) * px_per_frame - scroll_offset_;
            if (x < timeline_left - 1.0f || x > size.x + 1.0f) continue;
            const bool major = (frame % 10) == 0;
            const Color col = major ? Color(0.10f, 0.10f, 0.10f, 0.78f) : Color(0.46f, 0.46f, 0.46f, 0.42f);
            const float width = major ? 1.5f : 1.0f;
            draw_line(Vector2(x, ruler_y), Vector2(x, timeline_bottom), col, width);
            if (major && font.is_valid()) {
                const String label = String::num_int64(frame);
                draw_string(font,
                            Vector2(x - 8.0f, timeline_top + 16.0f),
                            label,
                            HORIZONTAL_ALIGNMENT_LEFT,
                            -1,
                            MAX(10, font_size - 1),
                            Color(0.82f, 0.82f, 0.82f, 1.0f));
            }
        }

        if (char_it != keyframes_.end()) {
            for (int row = 0; row < static_cast<int>(tracked_bones.size()); ++row) {
                const float center_y = ruler_y + static_cast<float>(row) * track_h + track_h * 0.5f;
                for (const auto& [frame, pose] : char_it->second) {
                    if (!pose_has_bone_for_visible_character(pose, tracked_bones[row])) continue;
                    const float x = timeline_left + CLAMP(static_cast<float>(frame), 0.0f, static_cast<float>(frame_count)) * px_per_frame - scroll_offset_;
                    if (x < timeline_left - 8.0f || x > size.x + 8.0f) continue;
                    const bool is_focus_key =
                        (frame == selected_keyframe_frame_) &&
                        !selected_keyframe_bone_.is_empty() &&
                        tracked_bones[row].nocasecmp_to(selected_keyframe_bone_) == 0;
                    const float hs = is_focus_key ? 6.5f : 5.0f;
                    PackedVector2Array pts;
                    pts.push_back(Vector2(x, center_y - hs));
                    pts.push_back(Vector2(x + hs, center_y));
                    pts.push_back(Vector2(x, center_y + hs));
                    pts.push_back(Vector2(x - hs, center_y));
                    PackedColorArray cols;
                    cols.push_back(is_focus_key
                        ? Color(1.0f, 0.90f, 0.30f, 1.0f)
                        : Color(0.95f, 0.78f, 0.10f, 1.0f));
                    draw_polygon(pts, cols);
                }
            }
        }

        const float playhead_x = timeline_left + CLAMP(static_cast<float>(current_frame_), 0.0f, static_cast<float>(frame_count)) * px_per_frame - scroll_offset_;
        if (playhead_x >= timeline_left - 1.0f && playhead_x <= size.x + 1.0f) {
            const Color playhead_color(1.0f, 0.43f, 0.24f, 1.0f);
            draw_line(Vector2(playhead_x, timeline_top), Vector2(playhead_x, timeline_bottom), playhead_color, 2.0f);
            draw_playhead_frame_badge(playhead_x, timeline_top, playhead_color, font, MAX(10, font_size - 1));
        }
    }

    void _gui_input(const Ref<InputEvent>& event) override {
        Ref<InputEventMouseButton> mb = event;
        if (mb.is_valid()) {
            const float toolbar_h = get_toolbar_height();
            if (mb->get_position().y <= toolbar_h) return;
            if (mb->get_button_index() == MOUSE_BUTTON_LEFT) {
                if (mb->is_pressed()) {
                    grab_focus();
                    int hit_frame = -1;
                    String hit_bone;
                    if (try_pick_keyframe_at(mb->get_position(), hit_frame, hit_bone)) {
                        selected_keyframe_frame_ = hit_frame;
                        selected_keyframe_bone_ = hit_bone;
                        queue_redraw();
                        accept_event();
                        return;
                    }
                    timeline_dragging_ = true;
                    seek_to_x(mb->get_position().x);
                    accept_event();
                    return;
                }
                timeline_dragging_ = false;
                accept_event();
                return;
            }
        }

        Ref<InputEventKey> key = event;
        if (key.is_valid() && key->is_pressed() && !key->is_echo()) {
            if (key->get_keycode() == Key::KEY_BACKSPACE || key->get_keycode() == Key::KEY_DELETE) {
                if (selected_keyframe_frame_ >= 0 && has_keyframe_at(selected_keyframe_frame_)) {
                    if (!selected_keyframe_bone_.is_empty()) {
                        remove_keyframe_bone(selected_keyframe_frame_, selected_keyframe_bone_);
                    } else {
                        remove_keyframe(selected_keyframe_frame_);
                    }
                    accept_event();
                    return;
                }
            }
        }

        Ref<InputEventMouseMotion> mm = event;
        if (!mm.is_valid() || !timeline_dragging_) return;
        if (!Input::get_singleton()->is_mouse_button_pressed(MOUSE_BUTTON_LEFT)) {
            timeline_dragging_ = false;
            return;
        }
        seek_to_x(mm->get_position().x);
        accept_event();
    }

    void set_fps(int value) { fps_ = value > 0 ? value : 1; }
    int get_fps() const { return fps_; }
    void set_total_frames(int value) {
        total_frames_ = value > 0 ? value : 1;
        refresh_scroll_range();
        queue_redraw();
    }
    int get_total_frames() const { return total_frames_; }
    void set_current_frame_prop(int value) { set_current_frame(value); }
    int get_current_frame_prop() const { return current_frame_; }

    void set_current_frame(int frame) {
        const int clamped = CLAMP(frame, 0, total_frames_);
        if (clamped == current_frame_) return;
        current_frame_ = clamped;
        update_frame_label();
        emit_signal("frameChanged", current_frame_);
        // Native fallback: apply current timeline pose directly to editor so
        // playback/scrubbing works even if SMS event routing is delayed.
        const Variant pose = get_pose_at_all_characters(current_frame_);
        if (pose.get_type() != Variant::NIL) {
            auto it = forge::SmsBridge::id_map().find("editor");
            if (it != forge::SmsBridge::id_map().end() && it->second != nullptr) {
                it->second->call("loadPose", pose);
            }
        }
        queue_redraw();
    }

    void set_keyframe(int frame, const Variant& pose) {
        set_keyframe_internal(frame, pose, String());
    }

    void set_keyframe_bone(int frame, const Variant& pose, const String& bone_name) {
        set_keyframe_internal(frame, pose, bone_name);
    }

    void set_keyframe_move(int frame) {
        auto it = forge::SmsBridge::id_map().find("editor");
        if (it == forge::SmsBridge::id_map().end() || it->second == nullptr) return;

        const Variant cid_variant = it->second->call("getActiveCharacterId");
        if (cid_variant.get_type() != Variant::STRING) return;
        const String cid = static_cast<String>(cid_variant);
        if (cid.is_empty()) return;

        const Variant count_variant = it->second->call("getSceneCharacterCount");
        if (count_variant.get_type() != Variant::INT) return;
        const int count = static_cast<int>(count_variant);

        int index = -1;
        for (int i = 0; i < count; ++i) {
            const Variant id_variant = it->second->call("getSceneCharacterId", i);
            if (id_variant.get_type() != Variant::STRING) continue;
            if (static_cast<String>(id_variant).nocasecmp_to(cid) == 0) {
                index = i;
                break;
            }
        }
        if (index < 0) return;

        const Variant x_var = it->second->call("getSceneCharacterPosX", index);
        const Variant y_var = it->second->call("getSceneCharacterPosY", index);
        const Variant z_var = it->second->call("getSceneCharacterPosZ", index);
        const bool valid_x = x_var.get_type() == Variant::INT || x_var.get_type() == Variant::FLOAT;
        const bool valid_y = y_var.get_type() == Variant::INT || y_var.get_type() == Variant::FLOAT;
        const bool valid_z = z_var.get_type() == Variant::INT || z_var.get_type() == Variant::FLOAT;
        if (!valid_x || !valid_y || !valid_z) return;

        Dictionary pos;
        pos["x"] = static_cast<double>(x_var);
        pos["y"] = static_cast<double>(y_var);
        pos["z"] = static_cast<double>(z_var);

        Dictionary payload;
        payload[cid + String(":@pos")] = pos;
        set_keyframe_internal(frame, payload, String("@pos"));
    }

    void set_keyframe_rotate(int frame) {
        auto it = forge::SmsBridge::id_map().find("editor");
        if (it == forge::SmsBridge::id_map().end() || it->second == nullptr) return;

        const Variant cid_variant = it->second->call("getActiveCharacterId");
        if (cid_variant.get_type() != Variant::STRING) return;
        const String cid = static_cast<String>(cid_variant);
        if (cid.is_empty()) return;

        const Variant count_variant = it->second->call("getSceneCharacterCount");
        if (count_variant.get_type() != Variant::INT) return;
        const int count = static_cast<int>(count_variant);

        int index = -1;
        for (int i = 0; i < count; ++i) {
            const Variant id_variant = it->second->call("getSceneCharacterId", i);
            if (id_variant.get_type() != Variant::STRING) continue;
            if (static_cast<String>(id_variant).nocasecmp_to(cid) == 0) {
                index = i;
                break;
            }
        }
        if (index < 0) return;

        const Variant x_var = it->second->call("getSceneCharacterRotX", index);
        const Variant y_var = it->second->call("getSceneCharacterRotY", index);
        const Variant z_var = it->second->call("getSceneCharacterRotZ", index);
        const bool valid_x = x_var.get_type() == Variant::INT || x_var.get_type() == Variant::FLOAT;
        const bool valid_y = y_var.get_type() == Variant::INT || y_var.get_type() == Variant::FLOAT;
        const bool valid_z = z_var.get_type() == Variant::INT || z_var.get_type() == Variant::FLOAT;
        if (!valid_x || !valid_y || !valid_z) return;

        Dictionary rot;
        rot["x"] = static_cast<double>(x_var);
        rot["y"] = static_cast<double>(y_var);
        rot["z"] = static_cast<double>(z_var);

        Dictionary payload;
        payload[cid + String(":@rot")] = rot;
        set_keyframe_internal(frame, payload, String("@rot"));
    }

private:
    void set_keyframe_internal(int frame, const Variant& pose, const String& bone_hint) {
        const std::string cid = current_character_key();
        const int clamped = CLAMP(frame, 0, total_frames_);
        Variant effective_pose = pose;
        Dictionary incoming_pose_dict;
        if (pose.get_type() == Variant::DICTIONARY) {
            incoming_pose_dict = static_cast<Dictionary>(pose);
        }
        if (effective_pose.get_type() == Variant::NIL) {
            auto it = forge::SmsBridge::id_map().find("editor");
            if (it != forge::SmsBridge::id_map().end() && it->second != nullptr) {
                const Variant from_editor = it->second->call("getPoseDataForActiveCharacter");
                if (from_editor.get_type() == Variant::DICTIONARY && !static_cast<Dictionary>(from_editor).is_empty()) {
                    effective_pose = from_editor;
                    UtilityFunctions::print("[ForgeRunner.Native] Timeline.setKeyframe used editor fallback pose.");
                }
            }
        }
        if (!bone_hint.is_empty() && effective_pose.get_type() == Variant::DICTIONARY) {
            const Dictionary src = static_cast<Dictionary>(effective_pose);
            Dictionary filtered;
            const Array keys = src.keys();
            for (int i = 0; i < keys.size(); ++i) {
                if (keys[i].get_type() != Variant::STRING) continue;
                const String key = static_cast<String>(keys[i]);
                String key_char;
                String key_bone;
                try_split_bone_key(key, key_char, key_bone);
                const String candidate = key_bone.is_empty() ? key : key_bone;
                if (candidate.nocasecmp_to(bone_hint) != 0) continue;
                filtered[key] = src[key];
            }
            if (filtered.is_empty()) {
                return;
            }
            effective_pose = filtered;
            incoming_pose_dict = filtered;
        }
        if (effective_pose.get_type() == Variant::DICTIONARY) {
            const Dictionary delta = static_cast<Dictionary>(effective_pose);
            auto char_it = keyframes_.find(cid);
            if (char_it != keyframes_.end()) {
                auto frame_it = char_it->second.find(clamped);
                if (frame_it != char_it->second.end() && frame_it->second.get_type() == Variant::DICTIONARY) {
                    Dictionary merged = static_cast<Dictionary>(frame_it->second);
                    const Array keys = delta.keys();
                    for (int i = 0; i < keys.size(); ++i) {
                        if (keys[i].get_type() != Variant::STRING) continue;
                        const String key = static_cast<String>(keys[i]);
                        merged[key] = delta[key];
                    }
                    effective_pose = merged;
                }
            }
            keyframes_[cid][clamped] = effective_pose;
        } else {
            keyframes_[cid][clamped] = effective_pose;
        }
        int bone_count = 0;
        UtilityFunctions::print(String("[ForgeRunner.Native] Timeline.setKeyframe poseType=") + String::num_int64(effective_pose.get_type()));
        if (!incoming_pose_dict.is_empty()) {
            const Array keys = incoming_pose_dict.keys();
            for (int i = 0; i < keys.size(); ++i) {
                if (keys[i].get_type() != Variant::STRING) continue;
                const String k = static_cast<String>(keys[i]);
                const Variant v = incoming_pose_dict[k];
                UtilityFunctions::print(String("[ForgeRunner.Native] Timeline.setKeyframe deltaKey='") +
                                        k + "' valueType=" + String::num_int64(v.get_type()));
            }
        }
        if (effective_pose.get_type() == Variant::DICTIONARY) {
            const Dictionary d = static_cast<Dictionary>(effective_pose);
            bone_count = static_cast<int>(d.size());
        } else if (pose.get_type() != Variant::NIL) {
            bone_count = 1;
        }
        UtilityFunctions::print(
            String("[ForgeRunner.Native] Timeline.setKeyframe cid='") +
            String(cid.c_str()) + "' frame=" + String::num_int64(clamped) +
            " bones=" + String::num_int64(bone_count));
        refresh_scroll_range();
        queue_redraw();
        if (effective_pose.get_type() == Variant::DICTIONARY) {
            const Dictionary d = static_cast<Dictionary>(effective_pose);
            String signal_bone = "*";
            if (d.size() == 1) {
                const Array keys = d.keys();
                if (keys.size() == 1 && keys[0].get_type() == Variant::STRING) {
                    signal_bone = static_cast<String>(keys[0]);
                }
            }
            emit_signal("keyframeAdded", clamped, signal_bone);
        } else if (effective_pose.get_type() != Variant::NIL) {
            emit_signal("keyframeAdded", clamped, String("*"));
        }
    }

public:
    void remove_keyframe(int frame) {
        const std::string cid = current_character_key();
        auto char_it = keyframes_.find(cid);
        if (char_it == keyframes_.end()) return;
        const int clamped = CLAMP(frame, 0, total_frames_);
        if (char_it->second.erase(clamped) <= 0) return;
        if (char_it->second.empty()) {
            keyframes_.erase(char_it);
        }
        if (selected_keyframe_frame_ == clamped) {
            selected_keyframe_frame_ = -1;
            selected_keyframe_bone_ = String();
        }
        refresh_scroll_range();
        queue_redraw();
        emit_signal("keyframeRemoved", clamped);
    }

    void remove_keyframe_bone(int frame, const String& bone_name) {
        if (bone_name.is_empty()) {
            remove_keyframe(frame);
            return;
        }
        const std::string cid = current_character_key();
        auto char_it = keyframes_.find(cid);
        if (char_it == keyframes_.end()) return;
        const int clamped = CLAMP(frame, 0, total_frames_);
        auto frame_it = char_it->second.find(clamped);
        if (frame_it == char_it->second.end()) return;

        if (frame_it->second.get_type() != Variant::DICTIONARY) {
            remove_keyframe(clamped);
            return;
        }

        Dictionary pose = static_cast<Dictionary>(frame_it->second);
        const Array keys = pose.keys();
        bool removed = false;
        for (int i = 0; i < keys.size(); ++i) {
            if (keys[i].get_type() != Variant::STRING) continue;
            const String key = static_cast<String>(keys[i]);
            String key_char;
            String key_bone;
            try_split_bone_key(key, key_char, key_bone);

            if (!visible_character_id_.is_empty() && !key_char.is_empty() &&
                key_char.nocasecmp_to(visible_character_id_) != 0) {
                continue;
            }

            const String candidate = key_bone.is_empty() ? key : key_bone;
            if (candidate.nocasecmp_to(bone_name) != 0) continue;
            pose.erase(key);
            removed = true;
        }
        if (!removed) return;

        if (pose.is_empty()) {
            char_it->second.erase(frame_it);
            if (char_it->second.empty()) {
                keyframes_.erase(char_it);
            }
            selected_keyframe_frame_ = -1;
            selected_keyframe_bone_ = String();
        } else {
            frame_it->second = pose;
        }

        refresh_scroll_range();
        queue_redraw();
        emit_signal("keyframeRemoved", clamped);
    }

    bool has_keyframe_at(int frame) const {
        const std::string cid = current_character_key();
        auto char_it = keyframes_.find(cid);
        if (char_it == keyframes_.end()) return false;
        const int clamped = CLAMP(frame, 0, total_frames_);
        return char_it->second.find(clamped) != char_it->second.end();
    }

    static bool variant_to_quaternion(const Variant& v, Quaternion& out_q) {
        if (v.get_type() == Variant::QUATERNION) {
            out_q = static_cast<Quaternion>(v);
            return true;
        }
        if (v.get_type() != Variant::DICTIONARY) return false;
        const Dictionary d = static_cast<Dictionary>(v);
        const bool lower = d.has("x") && d.has("y") && d.has("z") && d.has("w");
        const bool upper = d.has("X") && d.has("Y") && d.has("Z") && d.has("W");
        if (!lower && !upper) return false;
        const char* kx = lower ? "x" : "X";
        const char* ky = lower ? "y" : "Y";
        const char* kz = lower ? "z" : "Z";
        const char* kw = lower ? "w" : "W";
        out_q = Quaternion(
            static_cast<double>(d[kx]),
            static_cast<double>(d[ky]),
            static_cast<double>(d[kz]),
            static_cast<double>(d[kw]));
        return true;
    }

    static Dictionary quaternion_to_dict(const Quaternion& q) {
        Dictionary d;
        d["x"] = q.x;
        d["y"] = q.y;
        d["z"] = q.z;
        d["w"] = q.w;
        return d;
    }

    static bool variant_pose_to_map(const Variant& pose, std::map<std::string, Quaternion>& out_map) {
        out_map.clear();
        if (pose.get_type() != Variant::DICTIONARY) return false;
        const Dictionary d = static_cast<Dictionary>(pose);
        const Array keys = d.keys();
        for (int i = 0; i < keys.size(); ++i) {
            if (keys[i].get_type() != Variant::STRING) continue;
            const String bone = static_cast<String>(keys[i]);
            Quaternion q;
            if (!variant_to_quaternion(d[bone], q)) continue;
            out_map[bone.utf8().get_data()] = q;
        }
        return !out_map.empty();
    }

    static Dictionary pose_map_to_variant_dict(const std::map<std::string, Quaternion>& pose_map) {
        Dictionary d;
        for (const auto& [bone, q] : pose_map) {
            d[String(bone.c_str())] = quaternion_to_dict(q);
        }
        return d;
    }

    Variant interpolate_pose(const Variant& prev_pose, const Variant& next_pose, float t) const {
        const Dictionary prev_dict = prev_pose.get_type() == Variant::DICTIONARY ? static_cast<Dictionary>(prev_pose) : Dictionary();
        const Dictionary next_dict = next_pose.get_type() == Variant::DICTIONARY ? static_cast<Dictionary>(next_pose) : Dictionary();
        if (prev_dict.is_empty() && next_dict.is_empty()) return prev_pose;

        std::set<std::string> all_keys;
        if (!prev_dict.is_empty()) {
            const Array prev_keys = prev_dict.keys();
            for (int i = 0; i < prev_keys.size(); ++i) {
                if (prev_keys[i].get_type() != Variant::STRING) continue;
                all_keys.insert(std::string(static_cast<String>(prev_keys[i]).utf8().get_data()));
            }
        }
        if (!next_dict.is_empty()) {
            const Array next_keys = next_dict.keys();
            for (int i = 0; i < next_keys.size(); ++i) {
                if (next_keys[i].get_type() != Variant::STRING) continue;
                all_keys.insert(std::string(static_cast<String>(next_keys[i]).utf8().get_data()));
            }
        }

        Dictionary out;
        const double tf = static_cast<double>(t);
        for (const auto& raw : all_keys) {
            const String key(raw.c_str());
            const bool has_prev = prev_dict.has(key);
            const bool has_next = next_dict.has(key);
            if (has_prev && has_next) {
                const Variant prev_v = prev_dict[key];
                const Variant next_v = next_dict[key];

                Quaternion pq;
                Quaternion nq;
                if (variant_to_quaternion(prev_v, pq) && variant_to_quaternion(next_v, nq)) {
                    out[key] = quaternion_to_dict(pq.slerp(nq, t));
                    continue;
                }

                String key_char;
                String key_bone;
                try_split_bone_key(key, key_char, key_bone);
                if ((key_bone == "@pos" || key_bone == "@rot") &&
                    prev_v.get_type() == Variant::DICTIONARY &&
                    next_v.get_type() == Variant::DICTIONARY) {
                    const Dictionary prev_pos = static_cast<Dictionary>(prev_v);
                    const Dictionary next_pos = static_cast<Dictionary>(next_v);
                    if (prev_pos.has("x") && prev_pos.has("y") && prev_pos.has("z") &&
                        next_pos.has("x") && next_pos.has("y") && next_pos.has("z")) {
                        const double px = static_cast<double>(prev_pos["x"]);
                        const double py = static_cast<double>(prev_pos["y"]);
                        const double pz = static_cast<double>(prev_pos["z"]);
                        const double nx = static_cast<double>(next_pos["x"]);
                        const double ny = static_cast<double>(next_pos["y"]);
                        const double nz = static_cast<double>(next_pos["z"]);
                        Dictionary lerped;
                        lerped["x"] = px + ((nx - px) * tf);
                        lerped["y"] = py + ((ny - py) * tf);
                        lerped["z"] = pz + ((nz - pz) * tf);
                        out[key] = lerped;
                        continue;
                    }
                }

                out[key] = prev_v;
                continue;
            }

            if (has_prev) {
                out[key] = prev_dict[key];
            } else if (has_next) {
                out[key] = next_dict[key];
            }
        }
        return out;
    }

    Variant get_pose_at(int frame) const {
        return get_pose_at_for_character_key(frame, current_character_key());
    }

    Variant get_pose_at_exact(int frame) const {
        return get_pose_at_exact_for_character_key(frame, current_character_key());
    }

    Variant get_pose_at_exact_for_character(int frame, const String& character_id) const {
        return get_pose_at_exact_for_character_key(frame, character_id.utf8().get_data());
    }

    Variant get_pose_at_for_character_key(int frame, const std::string& cid) const {
        auto char_it = keyframes_.find(cid);
        if (char_it == keyframes_.end()) return Variant();
        const auto& timeline = char_it->second;
        const int clamped = CLAMP(frame, 0, total_frames_);
        if (timeline.empty()) return Variant();
        std::set<std::string> all_keys;
        for (const auto& [_, pose] : timeline) {
            if (pose.get_type() != Variant::DICTIONARY) continue;
            const Dictionary d = static_cast<Dictionary>(pose);
            const Array keys = d.keys();
            for (int i = 0; i < keys.size(); ++i) {
                if (keys[i].get_type() != Variant::STRING) continue;
                all_keys.insert(std::string(static_cast<String>(keys[i]).utf8().get_data()));
            }
        }

        Dictionary out;
        for (const auto& raw_key : all_keys) {
            const String key(raw_key.c_str());
            bool has_prev = false;
            bool has_next = false;
            int prev_frame = 0;
            int next_frame = 0;
            Variant prev_value;
            Variant next_value;

            for (const auto& [kf_frame, pose] : timeline) {
                if (pose.get_type() != Variant::DICTIONARY) continue;
                const Dictionary d = static_cast<Dictionary>(pose);
                if (!d.has(key)) continue;

                if (kf_frame <= clamped && (!has_prev || kf_frame > prev_frame)) {
                    has_prev = true;
                    prev_frame = kf_frame;
                    prev_value = d[key];
                }
                if (kf_frame >= clamped && (!has_next || kf_frame < next_frame)) {
                    has_next = true;
                    next_frame = kf_frame;
                    next_value = d[key];
                }
            }

            if (!has_prev && !has_next) {
                continue;
            }
            if (has_prev && has_next && prev_frame != next_frame) {
                const int span = MAX(1, next_frame - prev_frame);
                const float t = static_cast<float>(clamped - prev_frame) / static_cast<float>(span);

                Quaternion pq;
                Quaternion nq;
                if (variant_to_quaternion(prev_value, pq) && variant_to_quaternion(next_value, nq)) {
                    out[key] = quaternion_to_dict(pq.slerp(nq, t));
                    continue;
                }

                String key_char;
                String key_bone;
                try_split_bone_key(key, key_char, key_bone);
                if ((key_bone == "@pos" || key_bone == "@rot") &&
                    prev_value.get_type() == Variant::DICTIONARY &&
                    next_value.get_type() == Variant::DICTIONARY) {
                    const Dictionary prev_pos = static_cast<Dictionary>(prev_value);
                    const Dictionary next_pos = static_cast<Dictionary>(next_value);
                    if (prev_pos.has("x") && prev_pos.has("y") && prev_pos.has("z") &&
                        next_pos.has("x") && next_pos.has("y") && next_pos.has("z")) {
                        const double px = static_cast<double>(prev_pos["x"]);
                        const double py = static_cast<double>(prev_pos["y"]);
                        const double pz = static_cast<double>(prev_pos["z"]);
                        const double nx = static_cast<double>(next_pos["x"]);
                        const double ny = static_cast<double>(next_pos["y"]);
                        const double nz = static_cast<double>(next_pos["z"]);
                        const double tf = static_cast<double>(t);
                        Dictionary lerped;
                        lerped["x"] = px + ((nx - px) * tf);
                        lerped["y"] = py + ((ny - py) * tf);
                        lerped["z"] = pz + ((nz - pz) * tf);
                        out[key] = lerped;
                        continue;
                    }
                }

                out[key] = prev_value;
                continue;
            }

            out[key] = has_prev ? prev_value : next_value;
        }

        if (!out.is_empty()) {
            return out;
        }

        auto exact = timeline.find(clamped);
        if (exact != timeline.end()) return exact->second;

        auto upper = timeline.upper_bound(clamped);
        if (upper == timeline.begin()) return upper->second;
        if (upper == timeline.end()) return std::prev(upper)->second;
        return std::prev(upper)->second;
    }

    Variant get_pose_at_exact_for_character_key(int frame, const std::string& cid) const {
        auto char_it = keyframes_.find(cid);
        if (char_it == keyframes_.end()) return Variant();
        const auto& timeline = char_it->second;
        const int clamped = CLAMP(frame, 0, total_frames_);
        auto exact = timeline.find(clamped);
        if (exact == timeline.end()) return Variant();
        return exact->second;
    }

    Variant get_pose_at_all_characters(int frame) const {
        Dictionary merged;
        for (const auto& [cid, _] : keyframes_) {
            const Variant pose = get_pose_at_for_character_key(frame, cid);
            if (pose.get_type() != Variant::DICTIONARY) continue;
            const Dictionary d = static_cast<Dictionary>(pose);
            const Array keys = d.keys();
            for (int i = 0; i < keys.size(); ++i) {
                if (keys[i].get_type() != Variant::STRING) continue;
                const String k = static_cast<String>(keys[i]);
                merged[k] = d[k];
            }
        }
        if (merged.is_empty()) return Variant();
        return merged;
    }

    void play() {
        if (is_playing_) return;
        is_playing_ = true;
        play_accumulated_ = 0.0f;
        update_play_button_text();
        emit_signal("playbackStarted");
    }

    void stop() {
        if (!is_playing_) return;
        is_playing_ = false;
        update_play_button_text();
        emit_signal("playbackStopped");
    }

    bool is_playing() const { return is_playing_; }

    void set_visible_character_id(const String& character_id) {
        visible_character_id_ = character_id;
        queue_redraw();
    }

    int get_keyframe_count() const {
        const String cid = visible_character_id_.is_empty() ? String("_default") : visible_character_id_;
        return get_keyframe_count_for_character(cid);
    }

    int get_keyframe_frame_at(int index) const {
        const String cid = visible_character_id_.is_empty() ? String("_default") : visible_character_id_;
        return get_keyframe_frame_at_for_character(index, cid);
    }

    int get_keyframe_count_for_character(const String& character_id) const {
        auto char_it = keyframes_.find(character_id.utf8().get_data());
        if (char_it == keyframes_.end()) return 0;
        return static_cast<int>(char_it->second.size());
    }

    int get_keyframe_frame_at_for_character(int index, const String& character_id) const {
        auto char_it = keyframes_.find(character_id.utf8().get_data());
        if (char_it == keyframes_.end() || index < 0 || index >= static_cast<int>(char_it->second.size())) return -1;
        int i = 0;
        for (const auto& [frame, _] : char_it->second) {
            if (i == index) return frame;
            ++i;
        }
        return -1;
    }

    int get_keyframe_bone_count_for_character(int frame, const String& character_id) const {
        auto char_it = keyframes_.find(character_id.utf8().get_data());
        if (char_it == keyframes_.end()) return 0;
        auto frame_it = char_it->second.find(frame);
        if (frame_it == char_it->second.end()) return 0;
        if (frame_it->second.get_type() == Variant::DICTIONARY) {
            return static_cast<int>(Dictionary(frame_it->second).size());
        }
        return 1;
    }

    void debug_log_keyframes_for_character(const String& character_id) const {
        UtilityFunctions::print(String("[ForgeRunner.Native] Timeline keyframes for '") + character_id + "': " + String::num_int64(get_keyframe_count_for_character(character_id)));
    }

    void clear_all_keyframes() {
        keyframes_.clear();
        selected_keyframe_frame_ = -1;
        selected_keyframe_bone_ = String();
        set_current_frame(0);
        refresh_scroll_range();
        queue_redraw();
    }

private:
    void ensure_timeline_ui() {
        if (toolbar_ != nullptr) return;

        toolbar_ = memnew(HBoxContainer);
        toolbar_->set_anchors_preset(Control::PRESET_TOP_WIDE);
        toolbar_->set_offset(Side::SIDE_LEFT, 0.0f);
        toolbar_->set_offset(Side::SIDE_TOP, 0.0f);
        toolbar_->set_offset(Side::SIDE_RIGHT, 0.0f);
        toolbar_->set_offset(Side::SIDE_BOTTOM, 28.0f);
        toolbar_->set_mouse_filter(Control::MOUSE_FILTER_STOP);
        add_child(toolbar_);

        play_button_ = memnew(Button);
        play_button_->set_text("Play");
        play_button_->set_custom_minimum_size(Vector2(56.0f, 24.0f));
        play_button_->connect("pressed", callable_mp(this, &ForgeTimelineControl::on_play_button_pressed));
        toolbar_->add_child(play_button_);

        stop_button_ = memnew(Button);
        stop_button_->set_text("Stop");
        stop_button_->set_custom_minimum_size(Vector2(56.0f, 24.0f));
        stop_button_->connect("pressed", callable_mp(this, &ForgeTimelineControl::on_stop_button_pressed));
        toolbar_->add_child(stop_button_);

        frame_label_ = memnew(Label);
        frame_label_->set_text("0 / 0");
        frame_label_->set_h_size_flags(Control::SIZE_EXPAND_FILL);
        toolbar_->add_child(frame_label_);

        update_play_button_text();

        scroll_bar_ = memnew(HScrollBar);
        scroll_bar_->set_name("TimelineScrollBar");
        scroll_bar_->set_anchors_preset(Control::PRESET_BOTTOM_WIDE);
        scroll_bar_->set_offset(Side::SIDE_LEFT, 0.0f);
        scroll_bar_->set_offset(Side::SIDE_TOP, -16.0f);
        scroll_bar_->set_offset(Side::SIDE_RIGHT, 0.0f);
        scroll_bar_->set_offset(Side::SIDE_BOTTOM, 0.0f);
        scroll_bar_->set_mouse_filter(Control::MOUSE_FILTER_STOP);
        scroll_bar_->connect("value_changed", callable_mp(this, &ForgeTimelineControl::on_scroll_changed));
        add_child(scroll_bar_);
    }

    void on_play_button_pressed() {
        if (is_playing_) stop();
        else play();
    }

    void on_stop_button_pressed() {
        stop();
        set_current_frame(0);
    }

    void seek_to_x(float x) {
        set_current_frame(frame_from_x(x));
    }

    int frame_from_x(float x) const {
        const float width = get_size().x;
        if (width <= 1.0f) return 0;
        const float timeline_left = 140.0f;
        const float px_per_frame = 8.0f;
        const float local_x = MAX(0.0f, x - timeline_left) + scroll_offset_;
        const int frame = static_cast<int>(std::round(local_x / px_per_frame));
        return CLAMP(frame, 0, total_frames_);
    }

    bool try_pick_keyframe_at(const Vector2& pos, int& out_frame, String& out_bone) const {
        out_frame = -1;
        out_bone = String();
        const Vector2 size = get_size();
        const float toolbar_h = get_toolbar_height();
        const float scrollbar_h = (scroll_bar_ != nullptr) ? MAX(12.0f, scroll_bar_->get_size().y) : 16.0f;
        const float timeline_bottom = size.y - scrollbar_h;
        const float bone_name_w = 140.0f;
        const float ruler_h = 24.0f;
        const float track_h = 22.0f;
        const float ruler_y = toolbar_h + ruler_h;
        const int frame_count = total_frames_ > 0 ? total_frames_ : 1;
        const float px_per_frame = 8.0f;

        if (pos.y < ruler_y || pos.y > timeline_bottom) return false;
        if (pos.x < bone_name_w) return false;

        const auto char_it = keyframes_.find(current_character_key());
        if (char_it == keyframes_.end()) return false;
        const std::vector<String> tracked_bones = get_tracked_bones_for_current_character();

        bool found = false;
        float best_dist = std::numeric_limits<float>::infinity();
        int best_frame = -1;
        String best_bone;
        for (int row = 0; row < static_cast<int>(tracked_bones.size()); ++row) {
            const float center_y = ruler_y + static_cast<float>(row) * track_h + track_h * 0.5f;
            for (const auto& [frame, pose] : char_it->second) {
                if (!pose_has_bone_for_visible_character(pose, tracked_bones[row])) continue;
                const float x = bone_name_w + CLAMP(static_cast<float>(frame), 0.0f, static_cast<float>(frame_count)) * px_per_frame - scroll_offset_;
                const float dist = Vector2(x, center_y).distance_to(pos);
                if (dist > 8.5f) continue;
                if (dist < best_dist) {
                    best_dist = dist;
                    best_frame = frame;
                    best_bone = tracked_bones[row];
                    found = true;
                }
            }
        }
        if (!found) return false;
        out_frame = best_frame;
        out_bone = best_bone;
        return true;
    }

    void on_scroll_changed(double value) {
        scroll_offset_ = static_cast<float>(value);
        queue_redraw();
    }

    void refresh_scroll_range() {
        if (scroll_bar_ == nullptr) return;
        const float width = get_size().x;
        const float timeline_left = 140.0f;
        const float timeline_w = MAX(1.0f, width - timeline_left);
        const float px_per_frame = 8.0f;
        const float content_w = static_cast<float>(MAX(1, total_frames_ + 1)) * px_per_frame + 40.0f;
        const float max_scroll = MAX(0.0f, content_w - timeline_w);

        scroll_bar_->set_min(0.0);
        scroll_bar_->set_max(max_scroll + timeline_w);
        scroll_bar_->set_page(timeline_w);
        const double clamped = CLAMP(scroll_bar_->get_value(), 0.0, static_cast<double>(max_scroll));
        if (std::abs(scroll_bar_->get_value() - clamped) > 0.001) {
            scroll_bar_->set_value(clamped);
        }
        scroll_offset_ = static_cast<float>(clamped);
    }

    static bool try_split_bone_key(const String& key, String& out_character_id, String& out_bone_name) {
        const int sep = key.find(":");
        if (sep <= 0 || sep >= key.length() - 1) {
            out_character_id = String();
            out_bone_name = key;
            return false;
        }
        out_character_id = key.substr(0, sep);
        out_bone_name = key.substr(sep + 1, key.length() - sep - 1);
        return true;
    }

    static String display_bone_name(const String& name) {
        if (name == "@pos") return "xyz";
        if (name == "@rot") return "rot";
        if (name.length() <= 16) return name;
        int idx = name.rfind("_");
        if (idx < 0) idx = name.rfind(":");
        if (idx >= 0 && idx < name.length() - 1) {
            return name.substr(idx + 1, name.length() - idx - 1);
        }
        return name;
    }

    std::vector<String> get_tracked_bones_for_current_character() const {
        std::vector<String> out;
        std::set<std::string> seen;

        const auto char_it = keyframes_.find(current_character_key());
        if (char_it == keyframes_.end()) return out;

        for (const auto& [_, pose] : char_it->second) {
            if (pose.get_type() != Variant::DICTIONARY) continue;
            const Dictionary d = static_cast<Dictionary>(pose);
            const Array keys = d.keys();
            for (int i = 0; i < keys.size(); ++i) {
                if (keys[i].get_type() != Variant::STRING) continue;
                const String key = static_cast<String>(keys[i]);
                String key_char;
                String key_bone;
                try_split_bone_key(key, key_char, key_bone);
                String use_name = key_bone;
                if (!visible_character_id_.is_empty() && !key_char.is_empty()) {
                    if (key_char.nocasecmp_to(visible_character_id_) != 0) continue;
                } else if (use_name.is_empty()) {
                    use_name = key;
                }
                if (use_name.is_empty()) continue;
                const std::string lowered = std::string(use_name.to_lower().utf8().get_data());
                if (seen.insert(lowered).second) {
                    out.push_back(use_name);
                }
            }
        }

        std::sort(out.begin(), out.end(), [](const String& a, const String& b) {
            return a.nocasecmp_to(b) < 0;
        });
        return out;
    }

    bool pose_has_bone_for_visible_character(const Variant& pose, const String& bone_name) const {
        if (pose.get_type() != Variant::DICTIONARY) return false;
        const Dictionary d = static_cast<Dictionary>(pose);
        const Array keys = d.keys();
        for (int i = 0; i < keys.size(); ++i) {
            if (keys[i].get_type() != Variant::STRING) continue;
            const String key = static_cast<String>(keys[i]);
            String key_char;
            String key_bone;
            try_split_bone_key(key, key_char, key_bone);

            if (!visible_character_id_.is_empty() && !key_char.is_empty() && key_char.nocasecmp_to(visible_character_id_) != 0) {
                continue;
            }

            const String candidate = key_bone.is_empty() ? key : key_bone;
            if (candidate.nocasecmp_to(bone_name) == 0) return true;
        }
        return false;
    }

    void update_frame_label() {
        if (frame_label_ == nullptr) return;
        frame_label_->set_text(String::num_int64(current_frame_) + " / " + String::num_int64(total_frames_));
    }

    void update_play_button_text() {
        if (play_button_ == nullptr) return;
        play_button_->set_text(is_playing_ ? String("Pause") : String("Play"));
    }

    void draw_playhead_frame_badge(float playhead_x, float timeline_top, const Color& color, const Ref<Font>& font, int font_size) {
        if (!font.is_valid()) return;
        const String label = String::num_int64(current_frame_);
        const Vector2 text_size = font->get_string_size(label, HORIZONTAL_ALIGNMENT_LEFT, -1, font_size);
        const float pad_x = 7.0f;
        const float pad_y = 2.0f;
        const Vector2 badge_size(text_size.x + pad_x * 2.0f, text_size.y + pad_y * 2.0f);
        const Vector2 badge_pos(playhead_x - badge_size.x * 0.5f, timeline_top + 3.0f);

        Ref<StyleBoxFlat> style;
        style.instantiate();
        style->set_bg_color(color);
        style->set_corner_radius_all(4);
        draw_style_box(style, Rect2(badge_pos, badge_size));

        draw_string(
            font,
            Vector2(badge_pos.x + pad_x, badge_pos.y + text_size.y - 3.0f),
            label,
            HORIZONTAL_ALIGNMENT_LEFT,
            -1,
            font_size,
            Color(1.0f, 1.0f, 1.0f, 1.0f));
    }

    float get_toolbar_height() const {
        return toolbar_ != nullptr ? toolbar_->get_size().y : 28.0f;
    }

    std::string current_character_key() const {
        return visible_character_id_.is_empty() ? std::string("_default") : std::string(visible_character_id_.utf8().get_data());
    }

    int fps_ = 24;
    int total_frames_ = 120;
    int current_frame_ = 0;
    int selected_keyframe_frame_ = -1;
    String selected_keyframe_bone_;
    bool is_playing_ = false;
    float play_accumulated_ = 0.0f;
    HBoxContainer* toolbar_ = nullptr;
    Button* play_button_ = nullptr;
    Button* stop_button_ = nullptr;
    Label* frame_label_ = nullptr;
    HScrollBar* scroll_bar_ = nullptr;
    bool timeline_dragging_ = false;
    float scroll_offset_ = 0.0f;
    String visible_character_id_;
    std::map<std::string, std::map<int, Variant>> keyframes_;
};

// ---------------------------------------------------------------------------
// ForgePosingEditorControl (native scaffold)
// ---------------------------------------------------------------------------

class ForgePosingEditorControl : public SubViewportContainer {
    GDCLASS(ForgePosingEditorControl, SubViewportContainer);

    struct SceneItem {
        String id;
        String name;
        String path;
        Node3D* node = nullptr;
        float px = 0.0f, py = 0.0f, pz = 0.0f;
        float rx = 0.0f, ry = 0.0f, rz = 0.0f;
        float sx = 1.0f, sy = 1.0f, sz = 1.0f;
        bool visible = true;
    };

protected:
    static void _bind_methods() {
        ClassDB::bind_method(D_METHOD("setMode", "mode"), &ForgePosingEditorControl::set_mode);
        ClassDB::bind_method(D_METHOD("setEditMode", "mode"), &ForgePosingEditorControl::set_edit_mode);
        ClassDB::bind_method(D_METHOD("setTransformSpace", "space"), &ForgePosingEditorControl::set_transform_space);
        ClassDB::bind_method(D_METHOD("setBoneTree", "id"), &ForgePosingEditorControl::set_bone_tree);
        ClassDB::bind_method(D_METHOD("setJointSpheresVisible", "visible"), &ForgePosingEditorControl::set_joint_spheres_visible);
        ClassDB::bind_method(D_METHOD("getSelectedBoneName"), &ForgePosingEditorControl::get_selected_bone_name);
        ClassDB::bind_method(D_METHOD("getSelectedBoneRotX"), &ForgePosingEditorControl::get_selected_bone_rot_x);
        ClassDB::bind_method(D_METHOD("getSelectedBoneRotY"), &ForgePosingEditorControl::get_selected_bone_rot_y);
        ClassDB::bind_method(D_METHOD("getSelectedBoneRotZ"), &ForgePosingEditorControl::get_selected_bone_rot_z);
        ClassDB::bind_method(D_METHOD("getSelectedBoneMinX"), &ForgePosingEditorControl::get_selected_bone_min_x);
        ClassDB::bind_method(D_METHOD("getSelectedBoneMinY"), &ForgePosingEditorControl::get_selected_bone_min_y);
        ClassDB::bind_method(D_METHOD("getSelectedBoneMinZ"), &ForgePosingEditorControl::get_selected_bone_min_z);
        ClassDB::bind_method(D_METHOD("getSelectedBoneMaxX"), &ForgePosingEditorControl::get_selected_bone_max_x);
        ClassDB::bind_method(D_METHOD("getSelectedBoneMaxY"), &ForgePosingEditorControl::get_selected_bone_max_y);
        ClassDB::bind_method(D_METHOD("getSelectedBoneMaxZ"), &ForgePosingEditorControl::get_selected_bone_max_z);
        ClassDB::bind_method(D_METHOD("setSelectedBoneRot", "x", "y", "z"), &ForgePosingEditorControl::set_selected_bone_rot);
        ClassDB::bind_method(D_METHOD("setSelectedBoneConstraint", "minX", "maxX", "minY", "maxY", "minZ", "maxZ"), &ForgePosingEditorControl::set_selected_bone_constraint);
        ClassDB::bind_method(D_METHOD("getPoseDataForActiveCharacter"), &ForgePosingEditorControl::get_pose_data_for_active_character);
        ClassDB::bind_method(D_METHOD("getPoseDataForBone", "boneName"), &ForgePosingEditorControl::get_pose_data_for_bone);
        ClassDB::bind_method(D_METHOD("getPoseDataForSelectedBone"), &ForgePosingEditorControl::get_pose_data_for_selected_bone);
        ClassDB::bind_method(D_METHOD("loadPose", "pose"), &ForgePosingEditorControl::load_pose);
        ClassDB::bind_method(D_METHOD("resetPose"), &ForgePosingEditorControl::reset_pose);
        ClassDB::bind_method(D_METHOD("getProjectText", "path"), &ForgePosingEditorControl::get_project_text);
        ClassDB::bind_method(D_METHOD("applyProjectText", "path", "text", "sync"), &ForgePosingEditorControl::apply_project_text);
        ClassDB::bind_method(D_METHOD("loadProject", "path"), &ForgePosingEditorControl::load_project);
        ClassDB::bind_method(D_METHOD("saveProject", "path"), &ForgePosingEditorControl::save_project);
        ClassDB::bind_method(D_METHOD("addSceneAsset", "path", "x", "y", "z"), &ForgePosingEditorControl::add_scene_asset);
        ClassDB::bind_method(D_METHOD("addGreyboxItem", "kind", "x", "y", "z"), &ForgePosingEditorControl::add_greybox_item);
        ClassDB::bind_method(D_METHOD("removeSceneCharacter", "index"), &ForgePosingEditorControl::remove_scene_character);
        ClassDB::bind_method(D_METHOD("removeSceneProp", "index"), &ForgePosingEditorControl::remove_scene_prop);
        ClassDB::bind_method(D_METHOD("selectSceneCharacter", "index"), &ForgePosingEditorControl::select_scene_character);
        ClassDB::bind_method(D_METHOD("selectSceneProp", "index"), &ForgePosingEditorControl::select_scene_prop);
        ClassDB::bind_method(D_METHOD("setSceneCharacterVisible", "index", "visible"), &ForgePosingEditorControl::set_scene_character_visible);
        ClassDB::bind_method(D_METHOD("setScenePropVisible", "index", "visible"), &ForgePosingEditorControl::set_scene_prop_visible);
        ClassDB::bind_method(D_METHOD("setSceneCharacterPos", "index", "x", "y", "z"), &ForgePosingEditorControl::set_scene_character_pos);
        ClassDB::bind_method(D_METHOD("setSceneCharacterRot", "index", "x", "y", "z"), &ForgePosingEditorControl::set_scene_character_rot);
        ClassDB::bind_method(D_METHOD("setSceneCharacterScale", "index", "x", "y", "z"), &ForgePosingEditorControl::set_scene_character_scale);
        ClassDB::bind_method(D_METHOD("setScenePropPos", "index", "x", "y", "z"), &ForgePosingEditorControl::set_scene_prop_pos);
        ClassDB::bind_method(D_METHOD("setScenePropRot", "index", "x", "y", "z"), &ForgePosingEditorControl::set_scene_prop_rot);
        ClassDB::bind_method(D_METHOD("setScenePropScale", "index", "x", "y", "z"), &ForgePosingEditorControl::set_scene_prop_scale);
        ClassDB::bind_method(D_METHOD("placeSelectedOnGround", "groundY"), &ForgePosingEditorControl::place_selected_on_ground);
        ClassDB::bind_method(D_METHOD("exportCurrentFramePng", "path"), &ForgePosingEditorControl::export_current_frame_png);
        ClassDB::bind_method(D_METHOD("exportFrameRangePng", "frameFrom", "frameTo", "outputDirectory"), &ForgePosingEditorControl::export_frame_range_png);
        ClassDB::bind_method(D_METHOD("startExportFrameRangePng", "frameFrom", "frameTo", "outputDirectory"), &ForgePosingEditorControl::start_export_frame_range_png);

        ClassDB::bind_method(D_METHOD("getSceneCharacterCount"), &ForgePosingEditorControl::get_scene_character_count);
        ClassDB::bind_method(D_METHOD("getScenePropCount"), &ForgePosingEditorControl::get_scene_prop_count);
        ClassDB::bind_method(D_METHOD("getSceneCharacterId", "index"), &ForgePosingEditorControl::get_scene_character_id);
        ClassDB::bind_method(D_METHOD("getActiveCharacterId"), &ForgePosingEditorControl::get_active_character_id);
        ClassDB::bind_method(D_METHOD("getSceneCharacterName", "index"), &ForgePosingEditorControl::get_scene_character_name);
        ClassDB::bind_method(D_METHOD("getScenePropName", "index"), &ForgePosingEditorControl::get_scene_prop_name);

        ClassDB::bind_method(D_METHOD("getSceneCharacterPosX", "index"), &ForgePosingEditorControl::get_scene_character_pos_x);
        ClassDB::bind_method(D_METHOD("getSceneCharacterPosY", "index"), &ForgePosingEditorControl::get_scene_character_pos_y);
        ClassDB::bind_method(D_METHOD("getSceneCharacterPosZ", "index"), &ForgePosingEditorControl::get_scene_character_pos_z);
        ClassDB::bind_method(D_METHOD("getSceneCharacterRotX", "index"), &ForgePosingEditorControl::get_scene_character_rot_x);
        ClassDB::bind_method(D_METHOD("getSceneCharacterRotY", "index"), &ForgePosingEditorControl::get_scene_character_rot_y);
        ClassDB::bind_method(D_METHOD("getSceneCharacterRotZ", "index"), &ForgePosingEditorControl::get_scene_character_rot_z);
        ClassDB::bind_method(D_METHOD("getSceneCharacterScaleX", "index"), &ForgePosingEditorControl::get_scene_character_scale_x);
        ClassDB::bind_method(D_METHOD("getSceneCharacterScaleY", "index"), &ForgePosingEditorControl::get_scene_character_scale_y);
        ClassDB::bind_method(D_METHOD("getSceneCharacterScaleZ", "index"), &ForgePosingEditorControl::get_scene_character_scale_z);
        ClassDB::bind_method(D_METHOD("getScenePropPosX", "index"), &ForgePosingEditorControl::get_scene_prop_pos_x);
        ClassDB::bind_method(D_METHOD("getScenePropPosY", "index"), &ForgePosingEditorControl::get_scene_prop_pos_y);
        ClassDB::bind_method(D_METHOD("getScenePropPosZ", "index"), &ForgePosingEditorControl::get_scene_prop_pos_z);
        ClassDB::bind_method(D_METHOD("getScenePropRotX", "index"), &ForgePosingEditorControl::get_scene_prop_rot_x);
        ClassDB::bind_method(D_METHOD("getScenePropRotY", "index"), &ForgePosingEditorControl::get_scene_prop_rot_y);
        ClassDB::bind_method(D_METHOD("getScenePropRotZ", "index"), &ForgePosingEditorControl::get_scene_prop_rot_z);
        ClassDB::bind_method(D_METHOD("getScenePropScaleX", "index"), &ForgePosingEditorControl::get_scene_prop_scale_x);
        ClassDB::bind_method(D_METHOD("getScenePropScaleY", "index"), &ForgePosingEditorControl::get_scene_prop_scale_y);
        ClassDB::bind_method(D_METHOD("getScenePropScaleZ", "index"), &ForgePosingEditorControl::get_scene_prop_scale_z);

        ClassDB::bind_method(D_METHOD("set_src", "path"), &ForgePosingEditorControl::set_src);
        ClassDB::bind_method(D_METHOD("get_src"), &ForgePosingEditorControl::get_src);
        ClassDB::bind_method(D_METHOD("set_show_bone_tree", "value"), &ForgePosingEditorControl::set_show_bone_tree);
        ClassDB::bind_method(D_METHOD("get_show_bone_tree"), &ForgePosingEditorControl::get_show_bone_tree);

        ADD_PROPERTY(PropertyInfo(Variant::STRING, "src"), "set_src", "get_src");
        ADD_PROPERTY(PropertyInfo(Variant::BOOL, "showBoneTree"), "set_show_bone_tree", "get_show_bone_tree");

        ADD_SIGNAL(MethodInfo("boneSelected", PropertyInfo(Variant::STRING, "boneName")));
        ADD_SIGNAL(MethodInfo("poseChanged", PropertyInfo(Variant::STRING, "boneName")));
        ADD_SIGNAL(MethodInfo("poseReset"));
        ADD_SIGNAL(MethodInfo("scenePropAdded", PropertyInfo(Variant::INT, "index"), PropertyInfo(Variant::STRING, "path")));
        ADD_SIGNAL(MethodInfo("scenePropRemoved", PropertyInfo(Variant::INT, "index")));
        ADD_SIGNAL(MethodInfo("objectSelected", PropertyInfo(Variant::INT, "propIdx")));
        ADD_SIGNAL(MethodInfo("objectMoved", PropertyInfo(Variant::INT, "propIdx"), PropertyInfo(Variant::STRING, "pos")));
        ADD_SIGNAL(MethodInfo("exportProgress", PropertyInfo(Variant::STRING, "filename"), PropertyInfo(Variant::INT, "percent")));
        ADD_SIGNAL(MethodInfo("frameRangeExportFinished", PropertyInfo(Variant::INT, "written"), PropertyInfo(Variant::STRING, "outputDirectory")));
    }

public:
    void _ready() override {
        set_clip_contents(true);
        set_stretch(true);
        set_mouse_filter(Control::MOUSE_FILTER_STOP);
        ensure_viewport_scene();
        ensure_gizmo_nodes();
        update_camera_transform();
        update_gizmo_visual();
    }

    void _process(double) override {
        process_frame_export_step();
        update_gizmo_visual();
    }

    void _notification(int) {}

    void _draw() override {}

    void _gui_input(const Ref<InputEvent>& event) override {
        ensure_viewport_scene();
        if (camera_ == nullptr) return;

        Ref<InputEventMouseButton> mb = event;
        if (mb.is_valid()) {
            const int btn = mb->get_button_index();
            if (mb->is_pressed()) {
                if (btn == MOUSE_BUTTON_RIGHT) {
                    orbit_dragging_ = true;
                    drag_last_mouse_ = mb->get_position();
                    accept_event();
                    return;
                }
                if (btn == MOUSE_BUTTON_MIDDLE) {
                    pan_dragging_ = true;
                    drag_last_mouse_ = mb->get_position();
                    accept_event();
                    return;
                }
                if (btn == MOUSE_BUTTON_LEFT) {
                    left_pressed_ = true;
                    left_press_pos_ = mb->get_position();
                    left_moved_ = false;
                    drag_last_mouse_ = mb->get_position();
                    if (mode_ == "pose") {
                        if (edit_mode_.to_lower() == "move") {
                            if (begin_arrange_transform_drag(mb->get_position())) {
                                left_moved_ = true;
                            }
                        } else if (!selected_bone_name_.is_empty() && begin_pose_rotate_drag(mb->get_position())) {
                            left_moved_ = true;
                        } else if (begin_arrange_transform_drag(mb->get_position())) {
                            // Pose rotate fallback: keep rotating selected item pivot until a bone is explicitly selected.
                            left_moved_ = true;
                        }
                    } else if (begin_arrange_transform_drag(mb->get_position())) {
                        left_moved_ = true;
                    }
                    accept_event();
                    return;
                }
                if (btn == MOUSE_BUTTON_WHEEL_UP) {
                    orbit_distance_ = MAX(1.0f, orbit_distance_ * 0.9f);
                    update_camera_transform();
                    accept_event();
                    return;
                }
                if (btn == MOUSE_BUTTON_WHEEL_DOWN) {
                    orbit_distance_ = MIN(200.0f, orbit_distance_ * 1.1f);
                    update_camera_transform();
                    accept_event();
                    return;
                }
            } else {
                if (btn == MOUSE_BUTTON_RIGHT) {
                    orbit_dragging_ = false;
                    accept_event();
                    return;
                }
                if (btn == MOUSE_BUTTON_MIDDLE) {
                    pan_dragging_ = false;
                    accept_event();
                    return;
                }
                if (btn == MOUSE_BUTTON_LEFT) {
                    if (active_drag_mode_ != DRAG_NONE) {
                        end_active_drag();
                        left_pressed_ = false;
                        accept_event();
                        return;
                    }
                    if (!left_moved_) {
                        pick_at_screen_pos(mb->get_position());
                    }
                    left_pressed_ = false;
                    accept_event();
                    return;
                }
            }
        }

        Ref<InputEventMouseMotion> mm = event;
        if (!mm.is_valid()) return;

        const Vector2 mouse_pos = mm->get_position();
        if (left_pressed_ && mouse_pos.distance_to(left_press_pos_) > 4.0f) {
            left_moved_ = true;
        }

        const Vector2 rel = mm->get_relative();
        if (left_pressed_ && active_drag_mode_ != DRAG_NONE) {
            update_active_drag(mouse_pos, rel);
            accept_event();
            return;
        }
        if (orbit_dragging_) {
            orbit_yaw_ -= rel.x * 0.01f;
            orbit_pitch_ = CLAMP(orbit_pitch_ + rel.y * 0.01f, -1.4f, 1.4f);
            update_camera_transform();
            accept_event();
            return;
        }
        if (pan_dragging_) {
            const Basis basis = camera_->get_global_transform().basis;
            const Vector3 right = basis.get_column(0);
            const Vector3 up = basis.get_column(1);
            const float pan_scale = MAX(0.0025f, orbit_distance_ * 0.0025f);
            orbit_target_ += (-right * rel.x + up * rel.y) * pan_scale;
            update_camera_transform();
            accept_event();
            return;
        }

        drag_last_mouse_ = mouse_pos;
    }

    void set_mode(const String& mode) {
        mode_ = mode.to_lower();
        if (active_drag_mode_ != DRAG_NONE) {
            end_active_drag();
        }
    }
    void set_edit_mode(const String& mode) {
        edit_mode_ = mode.to_lower();
        if (active_drag_mode_ != DRAG_NONE && mode_ != "pose") {
            end_active_drag();
        }
    }
    void set_transform_space(const String& space) {
        transform_space_ = space.to_lower();
        if (active_drag_mode_ != DRAG_NONE && mode_ != "pose") {
            end_active_drag();
        }
    }
    void set_bone_tree(const String& id) {
        if (bone_tree_ != nullptr) {
            const Callable cb = callable_mp(this, &ForgePosingEditorControl::on_bone_tree_item_selected);
            if (bone_tree_->is_connected("item_selected", cb)) {
                bone_tree_->disconnect("item_selected", cb);
            }
            bone_tree_ = nullptr;
        }

        bone_tree_id_ = id;
        Tree* resolved = resolve_bone_tree_control();
        if (resolved == nullptr) return;

        bone_tree_ = resolved;
        const Callable cb = callable_mp(this, &ForgePosingEditorControl::on_bone_tree_item_selected);
        if (!bone_tree_->is_connected("item_selected", cb)) {
            bone_tree_->connect("item_selected", cb);
        }
        refresh_external_bone_tree();
    }
    void set_joint_spheres_visible(bool visible) { joint_spheres_visible_ = visible; }

    String get_selected_bone_name() const { return selected_bone_name_; }
    double get_selected_bone_rot_x() const { return selected_bone_rot_x_; }
    double get_selected_bone_rot_y() const { return selected_bone_rot_y_; }
    double get_selected_bone_rot_z() const { return selected_bone_rot_z_; }
    double get_selected_bone_min_x() const { return selected_bone_min_x_; }
    double get_selected_bone_min_y() const { return selected_bone_min_y_; }
    double get_selected_bone_min_z() const { return selected_bone_min_z_; }
    double get_selected_bone_max_x() const { return selected_bone_max_x_; }
    double get_selected_bone_max_y() const { return selected_bone_max_y_; }
    double get_selected_bone_max_z() const { return selected_bone_max_z_; }

    void set_selected_bone_rot(double x, double y, double z) {
        Skeleton3D* skel = nullptr;
        int bone_index = -1;
        if (!resolve_selected_bone(skel, bone_index)) return;

        selected_bone_rot_x_ = CLAMP(x, selected_bone_min_x_, selected_bone_max_x_);
        selected_bone_rot_y_ = CLAMP(y, selected_bone_min_y_, selected_bone_max_y_);
        selected_bone_rot_z_ = CLAMP(z, selected_bone_min_z_, selected_bone_max_z_);

        const Vector3 euler_rad(
            static_cast<float>(selected_bone_rot_x_ * (Math_PI / 180.0)),
            static_cast<float>(selected_bone_rot_y_ * (Math_PI / 180.0)),
            static_cast<float>(selected_bone_rot_z_ * (Math_PI / 180.0)));
        const Quaternion q = Quaternion::from_euler(euler_rad);
        skel->set_bone_pose_rotation(bone_index, q);
        const String bone_key = build_bone_key(get_active_character_id(), selected_bone_name_);
        if (!bone_key.is_empty()) {
            pose_data_[bone_key.utf8().get_data()] = q;
        }
        emit_signal("poseChanged", selected_bone_name_);
    }

    void set_selected_bone_constraint(double min_x, double max_x, double min_y, double max_y, double min_z, double max_z) {
        selected_bone_min_x_ = min_x; selected_bone_max_x_ = max_x;
        selected_bone_min_y_ = min_y; selected_bone_max_y_ = max_y;
        selected_bone_min_z_ = min_z; selected_bone_max_z_ = max_z;
    }

    Dictionary get_pose_data_for_active_character() const {
        String active_character_id = get_active_character_id();
        if (active_character_id.is_empty() && !characters_.empty()) {
            active_character_id = characters_[0].id;
        }
        Dictionary d;
        if (active_character_id.is_empty()) return d;

        const std::string prefix = std::string(active_character_id.utf8().get_data()) + ":";
        for (const auto& [bone_key, q] : pose_data_) {
            if (bone_key.rfind(prefix, 0) != 0) continue;
            d[String(bone_key.c_str())] = quaternion_to_dict(q);
        }

        if (d.is_empty() && !selected_bone_name_.is_empty()) {
            Skeleton3D* skel = nullptr;
            int bone_index = -1;
            if (resolve_selected_bone(skel, bone_index)) {
                const Quaternion q = skel->get_bone_pose_rotation(bone_index);
                const String key = build_bone_key(active_character_id, selected_bone_name_);
                if (!key.is_empty()) {
                    d[key] = quaternion_to_dict(q);
                }
            }
        }

        // Fallback: if cache-based data is still empty, export current active
        // skeleton pose directly so keyframe saves are never empty.
        if (d.is_empty()) {
            const int active_index = find_character_index_by_id(active_character_id);
            if (active_index >= 0) {
                const SceneItem* ch = &characters_[active_index];
                if (ch != nullptr && ch->node != nullptr) {
                    Skeleton3D* skel = find_first_skeleton(ch->node);
                    if (skel != nullptr) {
                        const int bone_count = skel->get_bone_count();
                        for (int i = 0; i < bone_count; ++i) {
                            const String bone_name = skel->get_bone_name(i);
                            if (bone_name.is_empty()) continue;
                            const Quaternion q = skel->get_bone_pose_rotation(i);
                            const String key = build_bone_key(active_character_id, bone_name);
                            if (!key.is_empty()) {
                                d[key] = quaternion_to_dict(q);
                            }
                        }
                    }
                }
            }
        }
        UtilityFunctions::print(String("[ForgeRunner.Native] getPoseDataForActiveCharacter id='") +
                                active_character_id + "' bones=" + String::num_int64(d.size()));
        return d;
    }

    Dictionary get_pose_data_for_selected_bone() const {
        return get_pose_data_for_bone(selected_bone_name_);
    }

    Dictionary get_pose_data_for_bone(const String& bone_name) const {
        Dictionary d;
        if (bone_name.is_empty()) return d;

        String active_character_id = get_active_character_id();
        if (active_character_id.is_empty() && !characters_.empty()) {
            active_character_id = characters_[0].id;
        }
        if (active_character_id.is_empty()) return d;

        const String key = build_bone_key(active_character_id, bone_name);
        if (key.is_empty()) return d;

        const auto it = pose_data_.find(key.utf8().get_data());
        if (it != pose_data_.end()) {
            d[key] = quaternion_to_dict(it->second);
            return d;
        }

        if (selected_character_index_ < 0 || selected_character_index_ >= static_cast<int>(characters_.size())) return d;
        const SceneItem* character = &characters_[selected_character_index_];
        if (character == nullptr || character->node == nullptr) return d;
        Skeleton3D* skel = find_first_skeleton(character->node);
        if (skel == nullptr) return d;
        const int bone_index = skel->find_bone(bone_name);
        if (bone_index < 0) return d;
        const Quaternion q = skel->get_bone_pose_rotation(bone_index);
        d[key] = quaternion_to_dict(q);
        return d;
    }

    void load_pose(const Variant& pose) {
        if (pose.get_type() != Variant::DICTIONARY) return;
        const Dictionary pose_dict = static_cast<Dictionary>(pose);
        const Array keys = pose_dict.keys();
        for (int i = 0; i < keys.size(); ++i) {
            if (keys[i].get_type() != Variant::STRING) continue;
            const String bone_key = static_cast<String>(keys[i]);

            String character_id;
            String bone_name;
            if (!split_bone_key(bone_key, character_id, bone_name)) continue;

            const int char_index = find_character_index_by_id(character_id);
            if (char_index < 0) continue;
            SceneItem* character = at_char(char_index);
            if (character == nullptr || character->node == nullptr) continue;

            if (bone_name == "@pos") {
                if (pose_dict[bone_key].get_type() != Variant::DICTIONARY) continue;
                const Dictionary pos = static_cast<Dictionary>(pose_dict[bone_key]);
                if (!(pos.has("x") && pos.has("y") && pos.has("z"))) continue;
                const Vector3 next_pos(
                    static_cast<float>(static_cast<double>(pos["x"])),
                    static_cast<float>(static_cast<double>(pos["y"])),
                    static_cast<float>(static_cast<double>(pos["z"])));
                character->node->set_global_position(next_pos);
                sync_item_from_node(*character);
                if (char_index == selected_character_index_) {
                    update_selection_marker();
                }
                continue;
            }
            if (bone_name == "@rot") {
                if (pose_dict[bone_key].get_type() != Variant::DICTIONARY) continue;
                const Dictionary rot = static_cast<Dictionary>(pose_dict[bone_key]);
                if (!(rot.has("x") && rot.has("y") && rot.has("z"))) continue;
                const Vector3 next_rot(
                    static_cast<float>(static_cast<double>(rot["x"])),
                    static_cast<float>(static_cast<double>(rot["y"])),
                    static_cast<float>(static_cast<double>(rot["z"])));
                character->node->set_rotation_degrees(next_rot);
                sync_item_from_node(*character);
                if (char_index == selected_character_index_) {
                    update_selection_marker();
                }
                continue;
            }

            Skeleton3D* skel = find_first_skeleton(character->node);
            if (skel == nullptr) continue;
            const int bone_index = skel->find_bone(bone_name);
            if (bone_index < 0) continue;

            Quaternion q;
            if (!variant_to_quaternion(pose_dict[bone_key], q)) continue;
            skel->set_bone_pose_rotation(bone_index, q);
            pose_data_[bone_key.utf8().get_data()] = q;
        }
        update_selected_bone_rotation_cache();
    }
    void reset_pose() {
        for (SceneItem& character : characters_) {
            if (character.node == nullptr) continue;
            Skeleton3D* skel = find_first_skeleton(character.node);
            if (skel == nullptr) continue;
            const int bone_count = skel->get_bone_count();
            for (int i = 0; i < bone_count; ++i) {
                skel->reset_bone_pose(i);
            }
        }
        pose_data_.clear();
        selected_bone_rot_x_ = 0.0;
        selected_bone_rot_y_ = 0.0;
        selected_bone_rot_z_ = 0.0;
        emit_signal("poseReset");
    }

    String get_project_text(const String& path) const {
        return String(build_project_text(path).c_str());
    }

    bool apply_project_text(const String& path, const String& text, bool) {
        if (path.is_empty()) return false;
        Ref<FileAccess> file = FileAccess::open(path, FileAccess::WRITE);
        if (!file.is_valid()) {
            UtilityFunctions::push_warning(String("[ForgeRunner.Native] applyProjectText failed to open: ") + path);
            return false;
        }
        file->store_string(text);
        file.unref();
        last_project_path_ = path;
        return load_project(path);
    }
    bool load_project(const String& path) {
        if (path.is_empty()) return false;
        last_project_path_ = path;
        UtilityFunctions::print(String("[ForgeRunner.Native] PosingEditor.loadProject: ") + path);
        ensure_viewport_scene();
        clear_scene_items();
        ForgeTimelineControl* timeline = resolve_timeline_control();
        if (timeline != nullptr) {
            timeline->clear_all_keyframes();
        }

        Ref<FileAccess> file = FileAccess::open(path, FileAccess::READ);
        if (!file.is_valid()) {
            UtilityFunctions::push_warning(String("[ForgeRunner.Native] loadProject failed to open: ") + path);
            return false;
        }

        std::string source = file->get_as_text().utf8().get_data();
        if (source.size() >= 3 &&
            static_cast<unsigned char>(source[0]) == 0xEF &&
            static_cast<unsigned char>(source[1]) == 0xBB &&
            static_cast<unsigned char>(source[2]) == 0xBF) {
            source.erase(0, 3);
        }
        smlcore::Document doc;
        try {
            doc = smlcore::parse_document(source);
        } catch (const std::exception& ex) {
            UtilityFunctions::push_warning(String("[ForgeRunner.Native] loadProject parse error: ") + String(ex.what()));
            return false;
        } catch (...) {
            UtilityFunctions::push_warning("[ForgeRunner.Native] loadProject parse error.");
            return false;
        }

        const smlcore::Node* scene = nullptr;
        for (const auto& root : doc.roots) {
            std::string name = root.name;
            std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
            if (name == "scene") {
                scene = &root;
                break;
            }
        }
        if (scene == nullptr) return false;
        scene_properties_.clear();
        for (const auto& prop : scene->properties) {
            scene_properties_[prop.name] = prop.value;
        }

        const std::string project_path = path.utf8().get_data();
        for (const auto& child : scene->children) {
            std::string child_name = child.name;
            std::transform(child_name.begin(), child_name.end(), child_name.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); });

            if (child_name == "character") {
                SceneItem item;
                item.id = String(child.get_value("id", ("char_" + std::to_string(characters_.size())).c_str()).c_str());
                item.name = String(child.get_value("name", "Character").c_str());
                item.path = String(resolve_project_asset_path(project_path, child.get_value("src", "")).c_str());
                parse_vec3(child.get_value("pos", "0,0,0"), item.px, item.py, item.pz);
                parse_vec3(child.get_value("rot", "0,0,0"), item.rx, item.ry, item.rz);
                parse_vec3(child.get_value("scale", "1,1,1"), item.sx, item.sy, item.sz);
                item.node = load_scene_node(item.path);
                if (item.node != nullptr) {
                    scene_root_->add_child(item.node);
                    apply_item_transform(item);
                    select_first_bone_from_node(item.node);
                } else {
                    UtilityFunctions::push_warning(String("[ForgeRunner.Native] Character node not loaded: ") + item.path);
                }
                characters_.push_back(item);

                if (timeline != nullptr) {
                    for (const auto& anim_child : child.children) {
                        std::string anim_name = anim_child.name;
                        std::transform(anim_name.begin(), anim_name.end(), anim_name.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
                        if (anim_name != "animation") continue;

                        for (const auto& key_node : anim_child.children) {
                            std::string key_name = key_node.name;
                            std::transform(key_name.begin(), key_name.end(), key_name.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
                            if (key_name != "key" && key_name != "keyframe") continue;

                            const int frame = parse_int_safe(key_node.get_value("frame", "-1"), -1);
                            if (frame < 0) continue;

                            Dictionary pose;
                            for (const auto& key_child : key_node.children) {
                                std::string key_child_name = key_child.name;
                                std::transform(key_child_name.begin(), key_child_name.end(), key_child_name.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
                                if (key_child_name == "bone") {
                                    const String bone_name(key_child.get_value("name", "").c_str());
                                    if (bone_name.is_empty()) continue;

                                    const double x = parse_double_safe(key_child.get_value("x", "0"), 0.0);
                                    const double y = parse_double_safe(key_child.get_value("y", "0"), 0.0);
                                    const double z = parse_double_safe(key_child.get_value("z", "0"), 0.0);
                                    const double w = parse_double_safe(key_child.get_value("w", "1"), 1.0);
                                    const Quaternion q(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), static_cast<float>(w));

                                    const String bone_key = build_bone_key(item.id, bone_name);
                                    if (!bone_key.is_empty()) {
                                        pose[bone_key] = quaternion_to_dict(q);
                                    }
                                    continue;
                                }
                                if (key_child_name == "move" || key_child_name == "position" || key_child_name == "pos") {
                                    Dictionary pos;
                                    pos["x"] = parse_double_safe(key_child.get_value("x", "0"), 0.0);
                                    pos["y"] = parse_double_safe(key_child.get_value("y", "0"), 0.0);
                                    pos["z"] = parse_double_safe(key_child.get_value("z", "0"), 0.0);
                                    pose[item.id + String(":@pos")] = pos;
                                    continue;
                                }
                                if (key_child_name == "rotate" || key_child_name == "rotation" || key_child_name == "rot") {
                                    Dictionary rot;
                                    rot["x"] = parse_double_safe(key_child.get_value("x", "0"), 0.0);
                                    rot["y"] = parse_double_safe(key_child.get_value("y", "0"), 0.0);
                                    rot["z"] = parse_double_safe(key_child.get_value("z", "0"), 0.0);
                                    pose[item.id + String(":@rot")] = rot;
                                }
                            }

                            if (!pose.is_empty()) {
                                timeline->set_visible_character_id(item.id);
                                timeline->set_keyframe(frame, pose);
                            }
                        }
                    }
                }
                continue;
            }

            if (child_name == "asset") {
                SceneItem item;
                item.id = String(child.get_value("id", ("prop_" + std::to_string(props_.size())).c_str()).c_str());
                item.name = String(child.get_value("name", "Asset").c_str());
                item.path = String(resolve_project_asset_path(project_path, child.get_value("src", "")).c_str());
                parse_vec3(child.get_value("pos", "0,0,0"), item.px, item.py, item.pz);
                parse_vec3(child.get_value("rot", "0,0,0"), item.rx, item.ry, item.rz);
                parse_vec3(child.get_value("scale", "1,1,1"), item.sx, item.sy, item.sz);
                item.node = load_scene_node(item.path);
                if (item.node != nullptr) {
                    scene_root_->add_child(item.node);
                    apply_item_transform(item);
                } else {
                    UtilityFunctions::push_warning(String("[ForgeRunner.Native] Asset node not loaded: ") + item.path);
                }
                props_.push_back(item);
            }
        }

        if (!characters_.empty()) {
            select_scene_character(0);
            if (timeline != nullptr) {
                timeline->set_visible_character_id(characters_[0].id);
            }
        }
        refresh_external_bone_tree();
        update_selection_marker();
        UtilityFunctions::print(String("[ForgeRunner.Native] PosingEditor.loadProject done: chars=") + String::num_int64(characters_.size()) + " props=" + String::num_int64(props_.size()));
        return true;
    }
    bool save_project(const String& path) {
        if (path.is_empty()) return false;
        last_project_path_ = path;
        UtilityFunctions::print(String("[ForgeRunner.Native] PosingEditor.saveProject: ") + path);
        if (ForgeTimelineControl* timeline = resolve_timeline_control()) {
            UtilityFunctions::print(String("[ForgeRunner.Native] saveProject timeline fps=") +
                                    String::num_int64(timeline->get_fps()) +
                                    " totalFrames=" + String::num_int64(timeline->get_total_frames()));
            for (size_t i = 0; i < characters_.size(); ++i) {
                const SceneItem& c = characters_[i];
                const int count = timeline->get_keyframe_count_for_character(c.id);
                UtilityFunctions::print(String("[ForgeRunner.Native] saveProject character '") +
                                        c.id + "' keyframes=" + String::num_int64(count));
            }
        } else {
            UtilityFunctions::push_warning("[ForgeRunner.Native] saveProject: timeline control not found.");
        }
        Ref<FileAccess> file = FileAccess::open(path, FileAccess::WRITE);
        if (!file.is_valid()) {
            UtilityFunctions::push_warning(String("[ForgeRunner.Native] saveProject failed to open: ") + path);
            return false;
        }
        file->store_string(get_project_text(path));
        return true;
    }

    bool export_current_frame_png(const String& path) {
        ensure_viewport_scene();
        if (sub_viewport_ == nullptr) {
            UtilityFunctions::push_warning("[ForgeRunner.Native] exportCurrentFramePng: viewport unavailable.");
            return false;
        }

        const String resolved = resolve_export_path(path, false);
        if (resolved.is_empty()) {
            UtilityFunctions::push_warning("[ForgeRunner.Native] exportCurrentFramePng: output path is empty.");
            return false;
        }

        const std::filesystem::path fs_path(resolved.utf8().get_data());
        const std::filesystem::path parent = fs_path.parent_path();
        if (!parent.empty()) {
            std::error_code ec;
            std::filesystem::create_directories(parent, ec);
            if (ec) {
                UtilityFunctions::push_warning(String("[ForgeRunner.Native] exportCurrentFramePng: could not create directory: ") + String(parent.string().c_str()));
                return false;
            }
        }

        const bool gizmo_was_visible = (gizmo_root_ != nullptr) && gizmo_root_->is_visible();
        const bool marker_was_visible = (selection_marker_ != nullptr) && selection_marker_->is_visible();
        if (gizmo_root_ != nullptr) {
            gizmo_root_->set_visible(false);
        }
        if (selection_marker_ != nullptr) {
            selection_marker_->set_visible(false);
        }
        clear_outline_materials();
        if (RenderingServer::get_singleton() != nullptr) {
            RenderingServer::get_singleton()->force_sync();
            RenderingServer::get_singleton()->force_draw(false, 0.0);
        }

        bool ok = false;
        Ref<ViewportTexture> texture = sub_viewport_->get_texture();
        if (!texture.is_valid()) {
            UtilityFunctions::push_warning("[ForgeRunner.Native] exportCurrentFramePng: viewport texture unavailable.");
        } else {
            Ref<Image> image = texture->get_image();
            if (!image.is_valid()) {
                UtilityFunctions::push_warning("[ForgeRunner.Native] exportCurrentFramePng: image capture failed.");
            } else {
                const Error err = image->save_png(resolved);
                if (err != OK) {
                    UtilityFunctions::push_warning(String("[ForgeRunner.Native] exportCurrentFramePng: save_png failed: ") + resolved);
                } else {
                    ok = true;
                }
            }
        }

        if (gizmo_was_visible) {
            update_gizmo_visual();
        } else if (gizmo_root_ != nullptr) {
            gizmo_root_->set_visible(false);
        }
        if (selection_marker_ != nullptr) {
            selection_marker_->set_visible(marker_was_visible);
        }
        update_selection_outline();
        return ok;
    }

    int export_frame_range_png(int frame_from, int frame_to, const String& output_directory) {
        ensure_viewport_scene();
        const String resolved_dir = resolve_export_path(output_directory, true);
        if (resolved_dir.is_empty()) {
            UtilityFunctions::push_warning("[ForgeRunner.Native] exportFrameRangePng: output directory is empty.");
            return 0;
        }

        std::error_code ec;
        std::filesystem::create_directories(std::filesystem::path(resolved_dir.utf8().get_data()), ec);
        if (ec) {
            UtilityFunctions::push_warning(String("[ForgeRunner.Native] exportFrameRangePng: could not create directory: ") + resolved_dir);
            return 0;
        }

        const int from = MAX(0, frame_from);
        const int to = MAX(from, frame_to);

        ForgeTimelineControl* timeline = resolve_timeline_control();
        int written = 0;
        for (int frame = from; frame <= to; ++frame) {
            if (timeline != nullptr) {
                timeline->set_current_frame(frame);
            }

            char name_buffer[32];
            std::snprintf(name_buffer, sizeof(name_buffer), "frame_%04d.png", frame - from);
            const String frame_name(name_buffer);
            const String frame_path = resolved_dir.path_join(frame_name);
            if (export_current_frame_png(frame_path)) {
                written += 1;
            }

            const int total = to - from + 1;
            const int percent = total > 0 ? static_cast<int>((static_cast<double>(frame - from + 1) * 100.0) / static_cast<double>(total)) : 100;
            emit_signal("exportProgress", frame_name, percent);
        }
        return written;
    }

    bool start_export_frame_range_png(int frame_from, int frame_to, const String& output_directory) {
        const String resolved_dir = resolve_export_path(output_directory, true);
        if (resolved_dir.is_empty()) {
            UtilityFunctions::push_warning("[ForgeRunner.Native] startExportFrameRangePng: output directory is empty.");
            return false;
        }
        if (frame_export_active_) {
            UtilityFunctions::push_warning("[ForgeRunner.Native] startExportFrameRangePng: export already active.");
            return false;
        }

        std::error_code ec;
        std::filesystem::create_directories(std::filesystem::path(resolved_dir.utf8().get_data()), ec);
        if (ec) {
            UtilityFunctions::push_warning(String("[ForgeRunner.Native] startExportFrameRangePng: could not create directory: ") + resolved_dir);
            return false;
        }

        frame_export_from_ = MAX(0, frame_from);
        frame_export_to_ = MAX(frame_export_from_, frame_to);
        frame_export_current_ = frame_export_from_;
        frame_export_written_ = 0;
        frame_export_output_dir_ = resolved_dir;
        frame_export_active_ = true;

        auto it = forge::SmsBridge::id_map().find("statusLabel");
        if (it != forge::SmsBridge::id_map().end()) {
            if (Label* status = Object::cast_to<Label>(it->second)) {
                status->set_text(String("Rendering frames ") + String::num_int64(frame_export_from_) + "..." + String::num_int64(frame_export_to_));
            }
        }
        return true;
    }

    int add_scene_asset(const String& path, double x, double y, double z) {
        ensure_viewport_scene();
        const bool as_character = characters_.empty();
        if (as_character) {
            SceneItem item;
            item.id = String("char_") + String::num_int64(characters_.size());
            item.name = String("Character ") + String::num_int64(characters_.size() + 1);
            item.path = path;
            item.px = static_cast<float>(x); item.py = static_cast<float>(y); item.pz = static_cast<float>(z);
            item.node = load_scene_node(path);
            if (item.node != nullptr) {
                scene_root_->add_child(item.node);
                apply_item_transform(item);
                select_first_bone_from_node(item.node);
            }
            characters_.push_back(item);
            selected_character_index_ = static_cast<int>(characters_.size()) - 1;
            selected_prop_index_ = -1;
            refresh_external_bone_tree();
            update_selection_marker();
            emit_signal("objectSelected", -1);
            return 1;
        }

        SceneItem prop;
        prop.id = String("prop_") + String::num_int64(props_.size());
        prop.name = String("Prop ") + String::num_int64(props_.size() + 1);
        prop.path = path;
        prop.px = static_cast<float>(x); prop.py = static_cast<float>(y); prop.pz = static_cast<float>(z);
        prop.node = load_scene_node(path);
        if (prop.node != nullptr) {
            scene_root_->add_child(prop.node);
            apply_item_transform(prop);
        }
        props_.push_back(prop);
        const int idx = static_cast<int>(props_.size()) - 1;
        selected_prop_index_ = idx;
        selected_character_index_ = -1;
        update_selection_marker();
        emit_signal("scenePropAdded", idx, path);
        emit_signal("objectSelected", idx);
        return 0;
    }

    int add_greybox_item(const String&, double x, double y, double z) {
        ensure_viewport_scene();
        SceneItem prop;
        prop.id = String("prop_") + String::num_int64(props_.size());
        prop.name = String("Greybox ") + String::num_int64(props_.size() + 1);
        prop.px = static_cast<float>(x); prop.py = static_cast<float>(y); prop.pz = static_cast<float>(z);
        auto* mesh = memnew(MeshInstance3D);
        Ref<BoxMesh> box;
        box.instantiate();
        box->set_size(Vector3(1.0f, 1.0f, 1.0f));
        mesh->set_mesh(box);
        Ref<StandardMaterial3D> mat;
        mat.instantiate();
        mat->set_albedo(Color(0.62f, 0.62f, 0.65f, 1.0f));
        mesh->set_material_override(mat);
        prop.node = mesh;
        if (prop.node != nullptr) {
            scene_root_->add_child(prop.node);
            apply_item_transform(prop);
        }
        props_.push_back(prop);
        const int idx = static_cast<int>(props_.size()) - 1;
        emit_signal("scenePropAdded", idx, String("greybox"));
        return idx;
    }

    void remove_scene_character(int index) {
        if (index < 0 || index >= static_cast<int>(characters_.size())) return;
        const String removed_id = characters_[index].id;
        if (characters_[index].node != nullptr) {
            if (characters_[index].node->get_parent() != nullptr) characters_[index].node->get_parent()->remove_child(characters_[index].node);
            characters_[index].node->queue_free();
            characters_[index].node = nullptr;
        }
        characters_.erase(characters_.begin() + index);
        if (!removed_id.is_empty()) {
            const std::string prefix = std::string(removed_id.utf8().get_data()) + ":";
            std::vector<std::string> remove_keys;
            for (const auto& [k, _] : pose_data_) {
                if (k.rfind(prefix, 0) == 0) remove_keys.push_back(k);
            }
            for (const auto& k : remove_keys) pose_data_.erase(k);
        }
        if (selected_character_index_ == index) selected_character_index_ = -1;
        else if (selected_character_index_ > index) selected_character_index_ -= 1;
        refresh_external_bone_tree();
        update_selection_marker();
    }

    void remove_scene_prop(int index) {
        if (index < 0 || index >= static_cast<int>(props_.size())) return;
        if (props_[index].node != nullptr) {
            if (props_[index].node->get_parent() != nullptr) props_[index].node->get_parent()->remove_child(props_[index].node);
            props_[index].node->queue_free();
            props_[index].node = nullptr;
        }
        props_.erase(props_.begin() + index);
        emit_signal("scenePropRemoved", index);
        if (selected_prop_index_ == index) selected_prop_index_ = -1;
        else if (selected_prop_index_ > index) selected_prop_index_ -= 1;
        update_selection_marker();
    }

    void select_scene_character(int index) {
        if (index < 0 || index >= static_cast<int>(characters_.size())) return;
        selected_character_index_ = index;
        selected_prop_index_ = -1;
        if (characters_[index].node != nullptr) {
            select_first_bone_from_node(characters_[index].node);
        }
        refresh_external_bone_tree();
        update_selection_marker();
        emit_signal("objectSelected", -1);
    }

    void select_scene_prop(int index) {
        if (index < 0) {
            selected_prop_index_ = -1;
            selected_character_index_ = -1;
            refresh_external_bone_tree();
            update_selection_marker();
            emit_signal("objectSelected", -1);
            return;
        }
        if (index >= static_cast<int>(props_.size())) return;
        selected_prop_index_ = index;
        selected_character_index_ = -1;
        refresh_external_bone_tree();
        update_selection_marker();
        emit_signal("objectSelected", index);
    }

    void set_scene_character_visible(int index, bool visible) {
        if (auto* c = at_char(index)) {
            c->visible = visible;
            apply_item_transform(*c);
        }
    }
    void set_scene_prop_visible(int index, bool visible) {
        if (auto* p = at_prop(index)) {
            p->visible = visible;
            apply_item_transform(*p);
        }
    }
    void set_scene_character_pos(int index, double x, double y, double z) {
        if (auto* c = at_char(index)) {
            set_item_pos(c, x, y, z);
            apply_item_transform(*c);
            if (index == selected_character_index_) update_selection_marker();
        }
    }
    void set_scene_character_rot(int index, double x, double y, double z) {
        if (auto* c = at_char(index)) {
            set_item_rot(c, x, y, z);
            apply_item_transform(*c);
        }
    }
    void set_scene_character_scale(int index, double x, double y, double z) {
        if (auto* c = at_char(index)) {
            set_item_scale(c, x, y, z);
            apply_item_transform(*c);
        }
    }
    void set_scene_prop_pos(int index, double x, double y, double z) {
        if (auto* p = at_prop(index)) {
            set_item_pos(p, x, y, z);
            apply_item_transform(*p);
            if (index == selected_prop_index_) update_selection_marker();
            emit_prop_moved(index, *p);
        }
    }
    void set_scene_prop_rot(int index, double x, double y, double z) {
        if (auto* p = at_prop(index)) {
            set_item_rot(p, x, y, z);
            apply_item_transform(*p);
            emit_prop_moved(index, *p);
        }
    }
    void set_scene_prop_scale(int index, double x, double y, double z) {
        if (auto* p = at_prop(index)) {
            set_item_scale(p, x, y, z);
            apply_item_transform(*p);
            emit_prop_moved(index, *p);
        }
    }
    bool place_selected_on_ground(double ground_y) {
        if (selected_prop_index_ >= 0) {
            if (auto* p = at_prop(selected_prop_index_)) {
                p->py = static_cast<float>(ground_y);
                apply_item_transform(*p);
                update_selection_marker();
                emit_prop_moved(selected_prop_index_, *p);
            }
            return true;
        }
        if (selected_character_index_ >= 0) {
            if (auto* c = at_char(selected_character_index_)) {
                c->py = static_cast<float>(ground_y);
                apply_item_transform(*c);
                update_selection_marker();
                const String pos_json = String("{\"x\":") + String::num(c->px) + ",\"y\":" + String::num(c->py) + ",\"z\":" + String::num(c->pz) + "}";
                emit_signal("objectMoved", -1, pos_json);
            }
            return true;
        }
        return false;
    }

    int get_scene_character_count() const { return static_cast<int>(characters_.size()); }
    int get_scene_prop_count() const { return static_cast<int>(props_.size()); }
    String get_scene_character_id(int index) const { return (index >= 0 && index < static_cast<int>(characters_.size())) ? characters_[index].id : String(); }
    String get_active_character_id() const { return (selected_character_index_ >= 0 && selected_character_index_ < static_cast<int>(characters_.size())) ? characters_[selected_character_index_].id : String(); }
    String get_scene_character_name(int index) const { return (index >= 0 && index < static_cast<int>(characters_.size())) ? characters_[index].name : String(); }
    String get_scene_prop_name(int index) const { return (index >= 0 && index < static_cast<int>(props_.size())) ? props_[index].name : String(); }

    double get_scene_character_pos_x(int index) const { return get_num_char(index, &SceneItem::px); }
    double get_scene_character_pos_y(int index) const { return get_num_char(index, &SceneItem::py); }
    double get_scene_character_pos_z(int index) const { return get_num_char(index, &SceneItem::pz); }
    double get_scene_character_rot_x(int index) const { return get_num_char(index, &SceneItem::rx); }
    double get_scene_character_rot_y(int index) const { return get_num_char(index, &SceneItem::ry); }
    double get_scene_character_rot_z(int index) const { return get_num_char(index, &SceneItem::rz); }
    double get_scene_character_scale_x(int index) const { return get_num_char(index, &SceneItem::sx); }
    double get_scene_character_scale_y(int index) const { return get_num_char(index, &SceneItem::sy); }
    double get_scene_character_scale_z(int index) const { return get_num_char(index, &SceneItem::sz); }
    double get_scene_prop_pos_x(int index) const { return get_num_prop(index, &SceneItem::px); }
    double get_scene_prop_pos_y(int index) const { return get_num_prop(index, &SceneItem::py); }
    double get_scene_prop_pos_z(int index) const { return get_num_prop(index, &SceneItem::pz); }
    double get_scene_prop_rot_x(int index) const { return get_num_prop(index, &SceneItem::rx); }
    double get_scene_prop_rot_y(int index) const { return get_num_prop(index, &SceneItem::ry); }
    double get_scene_prop_rot_z(int index) const { return get_num_prop(index, &SceneItem::rz); }
    double get_scene_prop_scale_x(int index) const { return get_num_prop(index, &SceneItem::sx); }
    double get_scene_prop_scale_y(int index) const { return get_num_prop(index, &SceneItem::sy); }
    double get_scene_prop_scale_z(int index) const { return get_num_prop(index, &SceneItem::sz); }

    void set_src(const String& path) {
        src_ = path;
        if (src_.is_empty()) {
            return;
        }
        ensure_viewport_scene();
        if (characters_.empty()) {
            SceneItem item;
            item.id = "char_0";
            item.name = "Character 1";
            item.path = src_;
            item.node = load_scene_node(src_);
            if (item.node != nullptr) {
                scene_root_->add_child(item.node);
                apply_item_transform(item);
                select_first_bone_from_node(item.node);
            }
            characters_.push_back(item);
            selected_character_index_ = 0;
            selected_prop_index_ = -1;
            refresh_external_bone_tree();
            update_selection_marker();
            emit_signal("objectSelected", -1);
            return;
        }

        SceneItem& item = characters_[0];
        if (item.node != nullptr) {
            if (item.node->get_parent() != nullptr) item.node->get_parent()->remove_child(item.node);
            item.node->queue_free();
            item.node = nullptr;
        }
        item.path = src_;
        item.node = load_scene_node(src_);
        if (item.node != nullptr) {
            scene_root_->add_child(item.node);
            apply_item_transform(item);
            select_first_bone_from_node(item.node);
        }
        refresh_external_bone_tree();
        update_selection_marker();
    }
    String get_src() const { return src_; }
    void set_show_bone_tree(bool value) {
        show_bone_tree_ = value;
        if (show_bone_tree_) {
            refresh_external_bone_tree();
        }
    }
    bool get_show_bone_tree() const { return show_bone_tree_; }

private:
    static Dictionary quaternion_to_dict(const Quaternion& q) {
        Dictionary d;
        d["x"] = q.x;
        d["y"] = q.y;
        d["z"] = q.z;
        d["w"] = q.w;
        return d;
    }

    static bool variant_to_quaternion(const Variant& v, Quaternion& out_q) {
        if (v.get_type() == Variant::QUATERNION) {
            out_q = static_cast<Quaternion>(v);
            return true;
        }
        if (v.get_type() != Variant::DICTIONARY) return false;
        const Dictionary d = static_cast<Dictionary>(v);
        if (!d.has("x") || !d.has("y") || !d.has("z") || !d.has("w")) return false;
        out_q = Quaternion(
            static_cast<double>(d["x"]),
            static_cast<double>(d["y"]),
            static_cast<double>(d["z"]),
            static_cast<double>(d["w"]));
        return true;
    }

    static String build_bone_key(const String& character_id, const String& bone_name) {
        if (character_id.is_empty() || bone_name.is_empty()) return String();
        return character_id + String(":") + bone_name;
    }

    static bool split_bone_key(const String& key, String& out_character_id, String& out_bone_name) {
        const int sep = key.find(":");
        if (sep <= 0 || sep >= key.length() - 1) return false;
        out_character_id = key.substr(0, sep);
        out_bone_name = key.substr(sep + 1, key.length() - sep - 1);
        return !out_character_id.is_empty() && !out_bone_name.is_empty();
    }

    int find_character_index_by_id(const String& character_id) const {
        for (int i = 0; i < static_cast<int>(characters_.size()); ++i) {
            if (characters_[i].id == character_id) return i;
        }
        return -1;
    }

    bool resolve_selected_bone(Skeleton3D*& out_skeleton, int& out_bone_index) const {
        out_skeleton = nullptr;
        out_bone_index = -1;
        if (selected_character_index_ < 0 || selected_character_index_ >= static_cast<int>(characters_.size())) return false;
        const SceneItem* character = &characters_[selected_character_index_];
        if (character == nullptr || character->node == nullptr) return false;
        Skeleton3D* skel = find_first_skeleton(character->node);
        if (skel == nullptr || selected_bone_name_.is_empty()) return false;
        const int bone_index = skel->find_bone(selected_bone_name_);
        if (bone_index < 0) return false;
        out_skeleton = skel;
        out_bone_index = bone_index;
        return true;
    }

    static Vector3 quaternion_to_euler_degrees(const Quaternion& q) {
        const Vector3 e = q.get_euler();
        return Vector3(
            e.x * static_cast<float>(180.0 / Math_PI),
            e.y * static_cast<float>(180.0 / Math_PI),
            e.z * static_cast<float>(180.0 / Math_PI));
    }

    void update_selected_bone_rotation_cache() {
        Skeleton3D* skel = nullptr;
        int bone_index = -1;
        if (!resolve_selected_bone(skel, bone_index)) return;
        const Quaternion q = skel->get_bone_pose_rotation(bone_index);
        const Vector3 euler_deg = quaternion_to_euler_degrees(q);
        selected_bone_rot_x_ = euler_deg.x;
        selected_bone_rot_y_ = euler_deg.y;
        selected_bone_rot_z_ = euler_deg.z;
    }

    static int parse_int_safe(const std::string& value, int fallback) {
        try { return std::stoi(value); } catch (...) { return fallback; }
    }

    static double parse_double_safe(const std::string& value, double fallback) {
        try { return std::stod(value); } catch (...) { return fallback; }
    }

    static std::string sml_escape(const String& value) {
        const std::string raw = value.utf8().get_data();
        std::string out;
        out.reserve(raw.size() + 8);
        for (char c : raw) {
            if (c == '\\') out += "\\\\";
            else if (c == '"') out += "\\\"";
            else if (c == '\n') out += "\\n";
            else if (c == '\r') out += "\\r";
            else out += c;
        }
        return out;
    }

    static std::string fmt_num(float value) {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(7) << value;
        std::string out = ss.str();
        while (!out.empty() && out.back() == '0') out.pop_back();
        if (!out.empty() && out.back() == '.') out.pop_back();
        if (out == "-0") out = "0";
        if (out.empty()) out = "0";
        return out;
    }

    static std::string fmt_num_float_literal(float value) {
        std::string out = fmt_num(value);
        if (out.find('.') == std::string::npos) out += ".0";
        return out;
    }

    static std::string normalize_slashes(std::string value) {
        std::replace(value.begin(), value.end(), '\\', '/');
        return value;
    }

    static bool starts_with(const std::string& value, const std::string& prefix) {
        return value.size() >= prefix.size() && value.compare(0, prefix.size(), prefix) == 0;
    }

    static std::string dirname_from_path(const String& path) {
        return std::filesystem::path(path.utf8().get_data()).parent_path().string();
    }

    std::string serialize_scene_path(const String& raw_path, const String& project_path) const {
        const std::string raw = raw_path.utf8().get_data();
        if (raw.empty()) return raw;
        if (starts_with(raw, "builtin:")) return raw;
        if (starts_with(raw, "res:/") || starts_with(raw, "res://")) return raw;
        if (project_path.is_empty()) return raw;

        std::error_code ec;
        const std::filesystem::path base_path = std::filesystem::weakly_canonical(dirname_from_path(project_path), ec);
        if (ec) return raw;
        const std::filesystem::path abs_path = std::filesystem::weakly_canonical(raw, ec);
        if (ec) return raw;

        const std::string base = normalize_slashes(base_path.string());
        const std::string abs = normalize_slashes(abs_path.string());
        if (!starts_with(abs, base)) return raw;

        std::string rel = abs.substr(base.size());
        while (!rel.empty() && rel[0] == '/') rel.erase(rel.begin());
        return "res:/" + rel;
    }

    std::string build_project_text(const String& project_path) const {
        std::ostringstream out;
        out << "Scene {\n";

        int fps_value = 24;
        int total_frames_value = 120;
        const auto fps_it = scene_properties_.find("fps");
        const auto total_it = scene_properties_.find("totalFrames");
        if (fps_it != scene_properties_.end()) fps_value = parse_int_safe(fps_it->second, fps_value);
        if (total_it != scene_properties_.end()) total_frames_value = parse_int_safe(total_it->second, total_frames_value);
        if (ForgeTimelineControl* timeline = resolve_timeline_control()) {
            fps_value = timeline->get_fps();
            total_frames_value = timeline->get_total_frames();
        }
        out << "    fps: " << fps_value << "\n";
        out << "    totalFrames: " << total_frames_value << "\n";
        for (const auto& [key, value] : scene_properties_) {
            if (key == "fps" || key == "totalFrames") continue;
            out << "    " << key << ": \"" << sml_escape(String(value.c_str())) << "\"\n";
        }
        out << "\n";

        for (size_t i = 0; i < characters_.size(); ++i) {
            const SceneItem& c = characters_[i];
            const String id = c.id.is_empty() ? String(("char" + std::to_string(i + 1)).c_str()) : c.id;
            const String name = c.name.is_empty() ? String(("Character " + std::to_string(i + 1)).c_str()) : c.name;
            const std::string src = serialize_scene_path(c.path, project_path);

            out << "    Character {\n";
            out << "        id: " << id.utf8().get_data() << "  name: \"" << sml_escape(name) << "\"  src: \"" << sml_escape(String(src.c_str())) << "\"\n";
            out << "        pos: " << fmt_num(c.px) << ", " << fmt_num(c.py) << ", " << fmt_num(c.pz) << "\n";
            out << "        rot: " << fmt_num(c.rx) << ", " << fmt_num(c.ry) << ", " << fmt_num(c.rz) << "\n";
            out << "        scale: " << fmt_num(c.sx) << ", " << fmt_num(c.sy) << ", " << fmt_num(c.sz) << "\n";
            out << "\n";
            out << "        Animation {\n";
            int emitted_key_count = 0;
            if (ForgeTimelineControl* timeline = resolve_timeline_control()) {
                timeline->set_visible_character_id(c.id);
                const int keyframe_count = timeline->get_keyframe_count_for_character(c.id);
                for (int i = 0; i < keyframe_count; ++i) {
                    const int frame = timeline->get_keyframe_frame_at_for_character(i, c.id);
                    if (frame < 0) continue;
                    const Variant pose_var = timeline->get_pose_at_exact_for_character(frame, c.id);
                    if (pose_var.get_type() != Variant::DICTIONARY) continue;
                    const Dictionary pose = static_cast<Dictionary>(pose_var);
                    const Array keys = pose.keys();
                    bool wrote_any_bone = false;
                    std::ostringstream key_block;
                    for (int k = 0; k < keys.size(); ++k) {
                        if (keys[k].get_type() != Variant::STRING) continue;
                        const String bone_key = static_cast<String>(keys[k]);
                        String scoped_char_id;
                        String bone_name;
                        if (!split_bone_key(bone_key, scoped_char_id, bone_name)) {
                            // Legacy/unscoped key: treat as bone key for current character.
                            scoped_char_id = c.id;
                            bone_name = bone_key;
                        }
                        if (scoped_char_id != c.id || bone_name.is_empty()) continue;
                        if (bone_name == "@pos") {
                            if (pose[bone_key].get_type() != Variant::DICTIONARY) continue;
                            const Dictionary pos = static_cast<Dictionary>(pose[bone_key]);
                            if (!(pos.has("x") && pos.has("y") && pos.has("z"))) continue;
                            if (!wrote_any_bone) {
                                key_block << "            Key { frame: " << frame << "\n";
                            }
                            key_block << "                Move { x: " << fmt_num_float_literal(static_cast<float>(static_cast<double>(pos["x"])))
                                      << " y: " << fmt_num_float_literal(static_cast<float>(static_cast<double>(pos["y"])))
                                      << " z: " << fmt_num_float_literal(static_cast<float>(static_cast<double>(pos["z"])))
                                      << " }\n";
                            wrote_any_bone = true;
                            continue;
                        }
                        if (bone_name == "@rot") {
                            if (pose[bone_key].get_type() != Variant::DICTIONARY) continue;
                            const Dictionary rot = static_cast<Dictionary>(pose[bone_key]);
                            if (!(rot.has("x") && rot.has("y") && rot.has("z"))) continue;
                            if (!wrote_any_bone) {
                                key_block << "            Key { frame: " << frame << "\n";
                            }
                            key_block << "                Rotate { x: " << fmt_num_float_literal(static_cast<float>(static_cast<double>(rot["x"])))
                                      << " y: " << fmt_num_float_literal(static_cast<float>(static_cast<double>(rot["y"])))
                                      << " z: " << fmt_num_float_literal(static_cast<float>(static_cast<double>(rot["z"])))
                                      << " }\n";
                            wrote_any_bone = true;
                            continue;
                        }
                        Quaternion q;
                        if (!variant_to_quaternion(pose[bone_key], q)) continue;
                        if (!wrote_any_bone) {
                            key_block << "            Key { frame: " << frame << "\n";
                        }
                        key_block << "                Bone { name: \"" << sml_escape(bone_name)
                                  << "\" x: " << fmt_num_float_literal(q.x)
                                  << " y: " << fmt_num_float_literal(q.y)
                                  << " z: " << fmt_num_float_literal(q.z)
                                  << " w: " << fmt_num_float_literal(q.w)
                                  << " }\n";
                        wrote_any_bone = true;
                    }
                    if (wrote_any_bone) {
                        key_block << "            }\n";
                        out << key_block.str();
                        emitted_key_count += 1;
                    }
                }
            }
            out << "        }\n";
            UtilityFunctions::print(String("[ForgeRunner.Native] saveProject emit character '") +
                                    c.id + "' animationKeys=" + String::num_int64(emitted_key_count));
            out << "    }\n";
        }

        if (ForgeTimelineControl* timeline = resolve_timeline_control()) {
            const String active = get_active_character_id();
            timeline->set_visible_character_id(active);
        }

        for (size_t i = 0; i < props_.size(); ++i) {
            const SceneItem& p = props_[i];
            const String id = p.id.is_empty() ? String(("prop" + std::to_string(i)).c_str()) : p.id;
            const String name = p.name.is_empty() ? String(("Asset " + std::to_string(i + 1)).c_str()) : p.name;
            const std::string src = serialize_scene_path(p.path, project_path);

            out << "    Asset {\n";
            out << "        id: " << id.utf8().get_data() << "  name: \"" << sml_escape(name) << "\"  src: \"" << sml_escape(String(src.c_str())) << "\"\n";
            out << "        pos: " << fmt_num(p.px) << ", " << fmt_num(p.py) << ", " << fmt_num(p.pz) << "\n";
            out << "        rot: " << fmt_num(p.rx) << ", " << fmt_num(p.ry) << ", " << fmt_num(p.rz) << "\n";
            out << "        scale: " << fmt_num(p.sx) << ", " << fmt_num(p.sy) << ", " << fmt_num(p.sz) << "\n";
            out << "    }\n";
        }

        out << "}\n";
        return out.str();
    }

    static void parse_vec3(const std::string& value, float& x, float& y, float& z) {
        x = 0.0f; y = 0.0f; z = 0.0f;
        std::stringstream ss(value);
        std::string token;
        float* out[3] = {&x, &y, &z};
        int idx = 0;
        while (std::getline(ss, token, ',') && idx < 3) {
            try { *out[idx] = std::stof(token); } catch (...) {}
            ++idx;
        }
    }

    std::string resolve_appres_root() const {
        const char* appres = std::getenv("FORGE_RUNNER_APPRES_ROOT");
        if (appres != nullptr && appres[0] != '\0') {
            return appres;
        }
        const char* env = std::getenv("FORGE_RUNNER_URL");
        if (env == nullptr || env[0] == '\0') return {};
        std::string url(env);
        if (url.rfind("file://", 0) != 0) return {};
        std::string file_path = url.substr(7);
        return forge::dirname_copy(file_path);
    }

    std::string resolve_project_asset_path(const std::string& project_path, const std::string& raw_value) const {
        return forge::resolve_runtime_asset_path(
            raw_value,
            forge::dirname_copy(project_path),
            resolve_appres_root());
    }

    String resolve_export_path(const String& raw_path, bool treat_as_directory) const {
        const std::string raw = raw_path.utf8().get_data();
        if (raw.empty()) {
            return String();
        }

        std::string base_dir = ".";
        if (!last_project_path_.is_empty()) {
            base_dir = forge::dirname_copy(last_project_path_.utf8().get_data());
        }

        std::string resolved = forge::resolve_runtime_asset_path(raw, base_dir, resolve_appres_root());
        if (resolved.empty()) {
            return String();
        }

        // For file targets keep only parent directory creation later.
        if (!treat_as_directory) {
            return String(resolved.c_str());
        }

        return String(resolved.c_str());
    }

    void clear_scene_items() {
        for (auto& item : characters_) {
            if (item.node != nullptr) {
                if (item.node->get_parent() != nullptr) item.node->get_parent()->remove_child(item.node);
                item.node->queue_free();
                item.node = nullptr;
            }
        }
        for (auto& item : props_) {
            if (item.node != nullptr) {
                if (item.node->get_parent() != nullptr) item.node->get_parent()->remove_child(item.node);
                item.node->queue_free();
                item.node = nullptr;
            }
        }
        characters_.clear();
        props_.clear();
        pose_data_.clear();
        selected_character_index_ = -1;
        selected_prop_index_ = -1;
        refresh_external_bone_tree();
        update_selection_marker();
    }

    Tree* resolve_bone_tree_control() {
        if (bone_tree_id_.is_empty()) return nullptr;

        const std::string id = bone_tree_id_.utf8().get_data();
        auto it = forge::SmsBridge::id_map().find(id);
        if (it != forge::SmsBridge::id_map().end()) {
            if (Tree* mapped = Object::cast_to<Tree>(it->second)) {
                return mapped;
            }
        }

        SceneTree* tree = get_tree();
        if (tree == nullptr || tree->get_root() == nullptr) return nullptr;
        Node* found = tree->get_root()->find_child(bone_tree_id_, true, false);
        return Object::cast_to<Tree>(found);
    }

    ForgeTimelineControl* resolve_timeline_control() const {
        auto it = forge::SmsBridge::id_map().find("timeline");
        if (it == forge::SmsBridge::id_map().end() || it->second == nullptr) return nullptr;
        return Object::cast_to<ForgeTimelineControl>(it->second);
    }

    void process_frame_export_step() {
        if (!frame_export_active_) return;

        if (frame_export_current_ > frame_export_to_) {
            frame_export_active_ = false;
            UtilityFunctions::print(String("[ForgeRunner.Native] frame export finished: written=") +
                                    String::num_int64(frame_export_written_) +
                                    " dir=" + frame_export_output_dir_);
            emit_signal("frameRangeExportFinished", frame_export_written_, frame_export_output_dir_);
            return;
        }

        if (ForgeTimelineControl* timeline = resolve_timeline_control()) {
            timeline->set_current_frame(frame_export_current_);
        }

        char name_buffer[32];
        std::snprintf(name_buffer, sizeof(name_buffer), "frame_%04d.png", frame_export_current_ - frame_export_from_);
        const String frame_name(name_buffer);
        const String frame_path = frame_export_output_dir_.path_join(frame_name);
        if (export_current_frame_png(frame_path)) {
            frame_export_written_ += 1;
        }

        const int total = frame_export_to_ - frame_export_from_ + 1;
        const int done = frame_export_current_ - frame_export_from_ + 1;
        const int percent = total > 0 ? static_cast<int>((static_cast<double>(done) * 100.0) / static_cast<double>(total)) : 100;
        emit_signal("exportProgress", frame_name, percent);
        auto it = forge::SmsBridge::id_map().find("statusLabel");
        if (it != forge::SmsBridge::id_map().end()) {
            if (Label* status = Object::cast_to<Label>(it->second)) {
                status->set_text(String("Rendering frame ") +
                                 String::num_int64(frame_export_current_) + "/" +
                                 String::num_int64(frame_export_to_) + " (" +
                                 String::num_int64(percent) + "%)");
            }
        }

        frame_export_current_ += 1;
    }

    void refresh_external_bone_tree() {
        Tree* tree = bone_tree_;
        if (tree == nullptr) {
            tree = resolve_bone_tree_control();
            if (tree == nullptr) return;
            bone_tree_ = tree;
        }

        tree->clear();
        tree->set_hide_root(true);

        if (selected_character_index_ < 0 || selected_character_index_ >= static_cast<int>(characters_.size())) {
            return;
        }
        SceneItem* selected_char = at_char(selected_character_index_);
        if (selected_char == nullptr || selected_char->node == nullptr) return;

        Skeleton3D* skeleton = find_first_skeleton(selected_char->node);
        if (skeleton == nullptr) return;

        const int bone_count = skeleton->get_bone_count();
        if (bone_count <= 0) return;

        TreeItem* root = tree->create_item();
        std::vector<TreeItem*> items(static_cast<size_t>(bone_count), nullptr);
        TreeItem* selected_item = nullptr;

        for (int i = 0; i < bone_count; ++i) {
            const int parent_index = skeleton->get_bone_parent(i);
            TreeItem* parent = root;
            if (parent_index >= 0 && parent_index < bone_count && items[static_cast<size_t>(parent_index)] != nullptr) {
                parent = items[static_cast<size_t>(parent_index)];
            }

            TreeItem* item = tree->create_item(parent);
            const String bone_name = skeleton->get_bone_name(i);
            item->set_text(0, bone_name);
            item->set_metadata(0, bone_name);
            item->set_collapsed(parent_index >= 0);
            items[static_cast<size_t>(i)] = item;

            if (bone_name == selected_bone_name_) {
                selected_item = item;
            }
        }

        if (selected_item != nullptr) {
            suppress_bone_tree_item_selected_ = true;
            tree->set_selected(selected_item, 0);
            tree->scroll_to_item(selected_item);
            suppress_bone_tree_item_selected_ = false;
        }
    }

    void on_bone_tree_item_selected() {
        if (suppress_bone_tree_item_selected_) return;
        if (bone_tree_ == nullptr) return;
        TreeItem* selected = bone_tree_->get_selected();
        if (selected == nullptr) return;

        Variant value = selected->get_metadata(0);
        if (value.get_type() != Variant::STRING) return;
        handling_bone_tree_item_selected_ = true;
        select_bone_name(static_cast<String>(value));
        handling_bone_tree_item_selected_ = false;
    }

    void ensure_viewport_scene() {
        if (sub_viewport_ != nullptr) return;

        sub_viewport_ = memnew(SubViewport);
        sub_viewport_->set_name("PoserViewport");
        sub_viewport_->set_disable_3d(false);
        sub_viewport_->set_transparent_background(false);
        sub_viewport_->set_update_mode(SubViewport::UPDATE_ALWAYS);
        add_child(sub_viewport_);

        scene_root_ = memnew(Node3D);
        scene_root_->set_name("SceneRoot3D");
        sub_viewport_->add_child(scene_root_);

        camera_ = memnew(Camera3D);
        camera_->set_name("Camera3D");
        camera_->set_position(Vector3(0.0f, 2.0f, 4.5f));
        camera_->look_at_from_position(Vector3(0.0f, 2.0f, 4.5f), Vector3(0.0f, 1.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f));
        scene_root_->add_child(camera_);
        orbit_target_ = Vector3(0.0f, 1.0f, 0.0f);
        orbit_distance_ = camera_->get_position().distance_to(orbit_target_);

        light_ = memnew(DirectionalLight3D);
        light_->set_name("Sun");
        light_->set_rotation_degrees(Vector3(-45.0f, -35.0f, 0.0f));
        scene_root_->add_child(light_);

        ground_ = memnew(MeshInstance3D);
        ground_->set_name("Ground");
        Ref<PlaneMesh> plane;
        plane.instantiate();
        plane->set_size(Vector2(20.0f, 20.0f));
        ground_->set_mesh(plane);
        Ref<StandardMaterial3D> mat;
        mat.instantiate();
        mat->set_albedo(Color(0.22f, 0.24f, 0.27f, 1.0f));
        ground_->set_material_override(mat);
        scene_root_->add_child(ground_);

        ground_grid_ = memnew(MeshInstance3D);
        ground_grid_->set_name("GroundGrid");
        ground_grid_->set_mesh(build_ground_grid_mesh(20.0f, 40, 5));
        ground_grid_->set_position(Vector3(0.0f, 0.01f, 0.0f));
        scene_root_->add_child(ground_grid_);

        selection_marker_ = memnew(MeshInstance3D);
        selection_marker_->set_name("SelectionMarker");
        Ref<BoxMesh> marker_box;
        marker_box.instantiate();
        marker_box->set_size(Vector3(0.18f, 0.18f, 0.18f));
        selection_marker_->set_mesh(marker_box);
        Ref<StandardMaterial3D> marker_mat;
        marker_mat.instantiate();
        marker_mat->set_shading_mode(StandardMaterial3D::SHADING_MODE_UNSHADED);
        marker_mat->set_albedo(Color(0.97f, 0.67f, 0.18f, 0.95f));
        selection_marker_->set_material_override(marker_mat);
        selection_marker_->set_visible(false);
        scene_root_->add_child(selection_marker_);
    }

    Ref<ImmediateMesh> build_ground_grid_mesh(float size, int steps, int major_every) const {
        Ref<ImmediateMesh> mesh;
        mesh.instantiate();

        Ref<StandardMaterial3D> minor_mat;
        minor_mat.instantiate();
        minor_mat->set_shading_mode(StandardMaterial3D::SHADING_MODE_UNSHADED);
        minor_mat->set_albedo(Color(0.28f, 0.30f, 0.34f, 1.0f));

        Ref<StandardMaterial3D> major_mat;
        major_mat.instantiate();
        major_mat->set_shading_mode(StandardMaterial3D::SHADING_MODE_UNSHADED);
        major_mat->set_albedo(Color(0.42f, 0.45f, 0.50f, 1.0f));

        const float half = size * 0.5f;
        const int clamped_steps = MAX(2, steps);
        const float step = size / static_cast<float>(clamped_steps);
        const int major_step = MAX(1, major_every);

        auto emit_line_surface = [&](bool major) {
            mesh->surface_begin(Mesh::PRIMITIVE_LINES, major ? major_mat : minor_mat);
            for (int i = 0; i <= clamped_steps; ++i) {
                const float coord = -half + static_cast<float>(i) * step;
                const bool is_major = (i % major_step) == 0;
                if (is_major != major) continue;
                mesh->surface_add_vertex(Vector3(coord, 0.0f, -half));
                mesh->surface_add_vertex(Vector3(coord, 0.0f, half));
                mesh->surface_add_vertex(Vector3(-half, 0.0f, coord));
                mesh->surface_add_vertex(Vector3(half, 0.0f, coord));
            }
            mesh->surface_end();
        };

        emit_line_surface(false);
        emit_line_surface(true);
        return mesh;
    }

    void update_camera_transform() {
        if (camera_ == nullptr) return;
        const float cp = std::cos(orbit_pitch_);
        const float sp = std::sin(orbit_pitch_);
        const float cy = std::cos(orbit_yaw_);
        const float sy = std::sin(orbit_yaw_);
        const Vector3 offset(
            orbit_distance_ * cp * sy,
            orbit_distance_ * sp,
            orbit_distance_ * cp * cy);
        const Vector3 cam_pos = orbit_target_ + offset;
        camera_->set_position(cam_pos);
        camera_->look_at_from_position(cam_pos, orbit_target_, Vector3(0.0f, 1.0f, 0.0f));
    }

    void ensure_gizmo_nodes() {
        if (scene_root_ == nullptr) return;
        if (gizmo_root_ != nullptr) return;

        gizmo_root_ = memnew(Node3D);
        gizmo_root_->set_name("NativeTransformGizmo");
        gizmo_root_->set_visible(false);
        scene_root_->add_child(gizmo_root_);

        Ref<SphereMesh> sphere_tip_mesh;
        sphere_tip_mesh.instantiate();
        sphere_tip_mesh->set_radius(0.06f);
        sphere_tip_mesh->set_height(0.12f);

        Ref<BoxMesh> box_tip_mesh;
        box_tip_mesh.instantiate();
        box_tip_mesh->set_size(Vector3(0.09f, 0.09f, 0.09f));

        Ref<CylinderMesh> cone_tip_mesh;
        cone_tip_mesh.instantiate();
        cone_tip_mesh->set_top_radius(0.0f);
        cone_tip_mesh->set_bottom_radius(0.035f);
        cone_tip_mesh->set_height(0.14f);
        cone_tip_mesh->set_radial_segments(12);

        Ref<CylinderMesh> shaft_mesh;
        shaft_mesh.instantiate();
        shaft_mesh->set_top_radius(0.012f);
        shaft_mesh->set_bottom_radius(0.012f);
        shaft_mesh->set_height(0.40f);
        shaft_mesh->set_radial_segments(10);

        const Color axis_colors[3] = {
            Color(0.95f, 0.20f, 0.20f, 1.0f),
            Color(0.20f, 0.90f, 0.20f, 1.0f),
            Color(0.20f, 0.50f, 1.00f, 1.0f),
        };

        for (int axis = 0; axis < 3; ++axis) {
            Ref<StandardMaterial3D> mat;
            mat.instantiate();
            mat->set_shading_mode(StandardMaterial3D::SHADING_MODE_UNSHADED);
            mat->set_flag(BaseMaterial3D::FLAG_DISABLE_DEPTH_TEST, true);
            mat->set_albedo(axis_colors[axis]);
            gizmo_axis_materials_[axis] = mat;

            auto* shaft = memnew(MeshInstance3D);
            shaft->set_name(String("GizmoShaft") + String::num_int64(axis));
            shaft->set_mesh(shaft_mesh);
            shaft->set_material_override(mat);
            shaft->set_cast_shadows_setting(GeometryInstance3D::SHADOW_CASTING_SETTING_OFF);
            gizmo_root_->add_child(shaft);
            gizmo_axis_shafts_[axis] = shaft;

            auto* tip = memnew(MeshInstance3D);
            tip->set_name(String("GizmoTip") + String::num_int64(axis));
            tip->set_mesh(sphere_tip_mesh);
            tip->set_material_override(mat);
            tip->set_cast_shadows_setting(GeometryInstance3D::SHADOW_CASTING_SETTING_OFF);
            gizmo_root_->add_child(tip);
            gizmo_axis_tips_[axis] = tip;

            Ref<ImmediateMesh> axis_line;
            axis_line.instantiate();
            axis_line->surface_begin(Mesh::PRIMITIVE_LINES, mat);
            axis_line->surface_add_vertex(Vector3(-0.34f, 0.0f, 0.0f));
            axis_line->surface_add_vertex(Vector3(0.34f, 0.0f, 0.0f));
            axis_line->surface_end();
            auto* axis_line_mesh = memnew(MeshInstance3D);
            axis_line_mesh->set_name(String("GizmoAxisLine") + String::num_int64(axis));
            axis_line_mesh->set_mesh(axis_line);
            axis_line_mesh->set_cast_shadows_setting(GeometryInstance3D::SHADOW_CASTING_SETTING_OFF);
            if (axis == 0) axis_line_mesh->set_rotation_degrees(Vector3(0.0f, 0.0f, 0.0f));
            else if (axis == 1) axis_line_mesh->set_rotation_degrees(Vector3(0.0f, 0.0f, 90.0f));
            else axis_line_mesh->set_rotation_degrees(Vector3(0.0f, 90.0f, 0.0f));
            axis_line_mesh->set_visible(false);
            gizmo_root_->add_child(axis_line_mesh);
            gizmo_axis_lines_[axis] = axis_line_mesh;

            Ref<ImmediateMesh> ring_mesh;
            ring_mesh.instantiate();
            ring_mesh->surface_begin(Mesh::PRIMITIVE_LINES, mat);
            const float r = 0.35f;
            const int steps = 96;
            for (int i = 0; i < steps; ++i) {
                const float a0 = (static_cast<float>(i) / static_cast<float>(steps)) * static_cast<float>(Math_TAU);
                const float a1 = (static_cast<float>(i + 1) / static_cast<float>(steps)) * static_cast<float>(Math_TAU);
                ring_mesh->surface_add_vertex(Vector3(std::cos(a0) * r, std::sin(a0) * r, 0.0f));
                ring_mesh->surface_add_vertex(Vector3(std::cos(a1) * r, std::sin(a1) * r, 0.0f));
            }
            ring_mesh->surface_end();

            auto* ring = memnew(MeshInstance3D);
            ring->set_name(String("GizmoRing") + String::num_int64(axis));
            ring->set_mesh(ring_mesh);
            ring->set_cast_shadows_setting(GeometryInstance3D::SHADOW_CASTING_SETTING_OFF);
            if (axis == 0) ring->set_rotation_degrees(Vector3(0.0f, 90.0f, 0.0f));
            else if (axis == 1) ring->set_rotation_degrees(Vector3(90.0f, 0.0f, 0.0f));
            else ring->set_rotation_degrees(Vector3(0.0f, 0.0f, 0.0f));
            ring->set_visible(false);
            gizmo_root_->add_child(ring);
            gizmo_axis_rings_[axis] = ring;
        }

        gizmo_sphere_tip_mesh_ = sphere_tip_mesh;
        gizmo_box_tip_mesh_ = box_tip_mesh;
        gizmo_cone_tip_mesh_ = cone_tip_mesh;

        Ref<ImmediateMesh> diamond_mesh;
        diamond_mesh.instantiate();
        diamond_mesh->surface_begin(Mesh::PRIMITIVE_TRIANGLES);
        const Vector3 p0(0.0f, 0.030f, 0.0f);
        const Vector3 p1(0.030f, 0.0f, 0.0f);
        const Vector3 p2(0.0f, -0.030f, 0.0f);
        const Vector3 p3(-0.030f, 0.0f, 0.0f);
        const Vector3 p4(0.0f, 0.0f, 0.018f);
        const Vector3 p5(0.0f, 0.0f, -0.018f);
        auto add_tri = [&](const Vector3& a, const Vector3& b, const Vector3& c) {
            diamond_mesh->surface_add_vertex(a);
            diamond_mesh->surface_add_vertex(b);
            diamond_mesh->surface_add_vertex(c);
        };
        add_tri(p0, p1, p4); add_tri(p1, p2, p4); add_tri(p2, p3, p4); add_tri(p3, p0, p4);
        add_tri(p1, p0, p5); add_tri(p2, p1, p5); add_tri(p3, p2, p5); add_tri(p0, p3, p5);
        diamond_mesh->surface_end();
        gizmo_diamond_tip_mesh_ = diamond_mesh;

        auto* pivot = memnew(MeshInstance3D);
        pivot->set_name("GizmoPivot");
        Ref<SphereMesh> pivot_mesh;
        pivot_mesh.instantiate();
        pivot_mesh->set_radius(0.018f);
        pivot_mesh->set_height(0.036f);
        pivot->set_mesh(pivot_mesh);
        Ref<StandardMaterial3D> pivot_mat;
        pivot_mat.instantiate();
        pivot_mat->set_shading_mode(StandardMaterial3D::SHADING_MODE_UNSHADED);
        pivot_mat->set_flag(BaseMaterial3D::FLAG_DISABLE_DEPTH_TEST, true);
        pivot_mat->set_albedo(Color(0.96f, 0.96f, 0.98f, 0.92f));
        pivot->set_material_override(pivot_mat);
        pivot->set_cast_shadows_setting(GeometryInstance3D::SHADOW_CASTING_SETTING_OFF);
        pivot->set_visible(false);
        gizmo_root_->add_child(pivot);
        gizmo_pivot_ = pivot;
    }

    void update_gizmo_visual() {
        ensure_gizmo_nodes();
        if (gizmo_root_ == nullptr) return;

        SceneItem* item = nullptr;
        bool is_character = false;
        int index = -1;

        bool visible = false;
        Basis basis;
        Vector3 origin;

        const bool pose_mode = mode_.to_lower() == String("pose");
        const String pose_edit_mode = edit_mode_.to_lower();
        if (pose_mode) {
            if (pose_edit_mode == "move") {
                if (resolve_selected_scene_item(item, is_character, index) && item != nullptr && item->node != nullptr) {
                    origin = item->node->get_global_position();
                    basis = transform_space_local() ? item->node->get_global_transform().basis.orthonormalized() : Basis();
                    visible = true;
                }
            } else {
                Skeleton3D* skel = nullptr;
                int bone_index = -1;
                if (resolve_selected_bone(skel, bone_index) && skel != nullptr) {
                    const Transform3D bone_pose = skel->get_bone_global_pose(bone_index);
                    origin = skel->to_global(bone_pose.origin);
                    basis = Basis();
                    visible = true;
                } else if (resolve_selected_scene_item(item, is_character, index) && item != nullptr && item->node != nullptr) {
                    origin = item->node->get_global_position();
                    basis = transform_space_local() ? item->node->get_global_transform().basis.orthonormalized() : Basis();
                    visible = true;
                }
            }
        } else if (resolve_selected_scene_item(item, is_character, index) && item != nullptr && item->node != nullptr) {
            origin = item->node->get_global_position();
            basis = transform_space_local() ? item->node->get_global_transform().basis.orthonormalized() : Basis();
            visible = true;
        }

        gizmo_root_->set_visible(visible);
        if (!visible) return;

        gizmo_root_->set_global_transform(Transform3D(basis, origin));

        const String visual_mode = pose_mode
            ? (pose_edit_mode == "move" ? String("move") : String("rotate"))
            : edit_mode_.to_lower();
        const bool rotate_mode = visual_mode == "rotate";
        const bool scale_mode = visual_mode == "scale";
        const bool move_mode = !rotate_mode && !scale_mode;

        const float handle_dist = scale_mode ? 0.46f : 0.54f;
        const float diag = 0.35f * 0.70710677f;
        Vector3 tip_offsets[3] = {
            Vector3(handle_dist, 0.0f, 0.0f),
            Vector3(0.0f, handle_dist, 0.0f),
            Vector3(0.0f, 0.0f, -handle_dist),
        };
        if (rotate_mode) {
            tip_offsets[0] = Vector3(0.0f, -diag, diag);
            tip_offsets[1] = Vector3(diag, 0.0f, diag);
            tip_offsets[2] = Vector3(diag, diag, 0.0f);
        }

        const Color axis_colors[3] = {
            Color(0.95f, 0.20f, 0.20f, 1.0f),
            Color(0.20f, 0.90f, 0.20f, 1.0f),
            Color(0.20f, 0.50f, 1.00f, 1.0f),
        };
        const Color highlight(1.00f, 0.85f, 0.00f, 1.0f);

        for (int axis = 0; axis < 3; ++axis) {
            const bool axis_active = (active_drag_mode_ != DRAG_NONE && drag_axis_ == axis);
            const Color col = axis_active ? highlight : axis_colors[axis];
            if (gizmo_axis_materials_[axis].is_valid()) {
                gizmo_axis_materials_[axis]->set_albedo(col);
            }

            MeshInstance3D* shaft = gizmo_axis_shafts_[axis];
            MeshInstance3D* tip = gizmo_axis_tips_[axis];
            MeshInstance3D* ring = gizmo_axis_rings_[axis];
            MeshInstance3D* axis_line = gizmo_axis_lines_[axis];
            if (shaft != nullptr) {
                shaft->set_visible(!rotate_mode);
                if (!rotate_mode) {
                    if (axis == 0) {
                        shaft->set_position(Vector3(0.20f, 0.0f, 0.0f));
                        shaft->set_rotation_degrees(Vector3(0.0f, 0.0f, 90.0f));
                    } else if (axis == 1) {
                        shaft->set_position(Vector3(0.0f, 0.20f, 0.0f));
                        shaft->set_rotation_degrees(Vector3(0.0f, 0.0f, 0.0f));
                    } else {
                        shaft->set_position(Vector3(0.0f, 0.0f, -0.20f));
                        shaft->set_rotation_degrees(Vector3(-90.0f, 0.0f, 0.0f));
                    }
                }
            }
            if (tip != nullptr) {
                tip->set_position(tip_offsets[axis]);
                tip->set_visible(true);
                if (scale_mode && gizmo_box_tip_mesh_.is_valid()) {
                    tip->set_mesh(gizmo_box_tip_mesh_);
                    tip->set_scale(Vector3(1.0f, 1.0f, 1.0f));
                    tip->set_rotation_degrees(Vector3());
                } else if (move_mode && gizmo_cone_tip_mesh_.is_valid()) {
                    tip->set_mesh(gizmo_cone_tip_mesh_);
                    tip->set_scale(Vector3(1.0f, 1.0f, 1.0f));
                    if (axis == 0) tip->set_rotation_degrees(Vector3(0.0f, 0.0f, -90.0f));
                    else if (axis == 1) tip->set_rotation_degrees(Vector3(0.0f, 0.0f, 0.0f));
                    else tip->set_rotation_degrees(Vector3(-90.0f, 0.0f, 0.0f));
                } else if (rotate_mode && gizmo_diamond_tip_mesh_.is_valid()) {
                    tip->set_mesh(gizmo_diamond_tip_mesh_);
                    tip->set_scale(Vector3(1.0f, 1.0f, 1.0f));
                    if (axis == 0) tip->set_rotation_degrees(Vector3(90.0f, 0.0f, 45.0f));
                    else if (axis == 1) tip->set_rotation_degrees(Vector3(0.0f, 0.0f, -45.0f));
                    else tip->set_rotation_degrees(Vector3(0.0f, 0.0f, 45.0f));
                } else if (gizmo_sphere_tip_mesh_.is_valid()) {
                    tip->set_mesh(gizmo_sphere_tip_mesh_);
                    tip->set_scale(Vector3(1.0f, 1.0f, 1.0f));
                    tip->set_rotation_degrees(Vector3());
                }
            }
            if (ring != nullptr) {
                ring->set_visible(rotate_mode);
            }
            if (axis_line != nullptr) {
                axis_line->set_visible(rotate_mode);
            }
        }

        if (gizmo_pivot_ != nullptr) {
            gizmo_pivot_->set_visible(rotate_mode);
        }
    }

    static bool ray_sphere_intersect(
        const Vector3& origin,
        const Vector3& dir,
        const Vector3& center,
        float radius,
        float& out_t)
    {
        const Vector3 oc = origin - center;
        const float b = oc.dot(dir);
        const float c = oc.dot(oc) - radius * radius;
        const float d = b * b - c;
        if (d < 0.0f) return false;
        const float sq = std::sqrt(d);
        float t = -b - sq;
        if (t < 0.0f) t = -b + sq;
        if (t <= 0.0f) return false;
        out_t = t;
        return true;
    }

    static Vector3 axis_base_dir(int axis) {
        if (axis == 0) return Vector3(1.0f, 0.0f, 0.0f);
        if (axis == 1) return Vector3(0.0f, 1.0f, 0.0f);
        return Vector3(0.0f, 0.0f, 1.0f); // Back (+Z)
    }

    bool transform_space_local() const {
        return transform_space_.to_lower() == String("local");
    }

    bool resolve_selected_scene_item(SceneItem*& out_item, bool& out_is_character, int& out_index) {
        out_item = nullptr;
        out_is_character = false;
        out_index = -1;
        if (selected_character_index_ >= 0) {
            out_is_character = true;
            out_index = selected_character_index_;
            out_item = at_char(selected_character_index_);
            return out_item != nullptr && out_item->node != nullptr;
        }
        if (selected_prop_index_ >= 0) {
            out_is_character = false;
            out_index = selected_prop_index_;
            out_item = at_prop(selected_prop_index_);
            return out_item != nullptr && out_item->node != nullptr;
        }
        return false;
    }

    static void sync_item_from_node(SceneItem& item) {
        if (item.node == nullptr) return;
        const Vector3 p = item.node->get_global_position();
        const Vector3 r = item.node->get_rotation_degrees();
        const Vector3 s = item.node->get_scale();
        item.px = p.x; item.py = p.y; item.pz = p.z;
        item.rx = r.x; item.ry = r.y; item.rz = r.z;
        item.sx = s.x; item.sy = s.y; item.sz = s.z;
    }

    void emit_selected_object_moved() {
        if (drag_target_is_character_) {
            SceneItem* c = at_char(drag_target_index_);
            if (c == nullptr) return;
            sync_item_from_node(*c);
            const String pos_json = String("{\"x\":") + String::num(c->px) + ",\"y\":" + String::num(c->py) + ",\"z\":" + String::num(c->pz) + "}";
            emit_signal("objectMoved", -1, pos_json);
            return;
        }
        SceneItem* p = at_prop(drag_target_index_);
        if (p == nullptr) return;
        sync_item_from_node(*p);
        emit_prop_moved(drag_target_index_, *p);
    }

    Vector3 resolve_drag_world_axis(int axis, const SceneItem& item) const {
        const Vector3 base = axis_base_dir(axis);
        if (!transform_space_local() || item.node == nullptr) return base;
        Vector3 local_axis = item.node->get_global_transform().basis.xform(base);
        if (local_axis.length_squared() <= 1.0e-6f) return base;
        local_axis.normalize();
        return local_axis;
    }

    bool begin_arrange_transform_drag(const Vector2& screen_pos) {
        SceneItem* item = nullptr;
        bool is_character = false;
        int index = -1;
        if (!resolve_selected_scene_item(item, is_character, index)) return false;
        if (item == nullptr || item->node == nullptr) return false;

        const String mode = edit_mode_.to_lower();
        if (mode != "move" && mode != "scale" && mode != "rotate") return false;

        const Vector3 origin = item->node->get_global_position();
        const Basis basis = transform_space_local() ? item->node->get_global_transform().basis.orthonormalized() : Basis();
        const Vector3 ray_origin = camera_->project_ray_origin(screen_pos);
        const Vector3 ray_dir = camera_->project_ray_normal(screen_pos);

        float handle_dist = 0.54f; // Move default: shaft + cone
        float handle_pick_radius = 0.12f;
        Vector3 offsets[3] = {
            Vector3(handle_dist, 0.0f, 0.0f),
            Vector3(0.0f, handle_dist, 0.0f),
            Vector3(0.0f, 0.0f, -handle_dist),
        };

        if (mode == "scale") {
            handle_dist = 0.46f; // Scale: shaft + box
            offsets[0] = Vector3(handle_dist, 0.0f, 0.0f);
            offsets[1] = Vector3(0.0f, handle_dist, 0.0f);
            offsets[2] = Vector3(0.0f, 0.0f, -handle_dist);
        } else if (mode == "rotate") {
            const float diag = 0.35f * 0.70710677f;
            handle_pick_radius = 0.14f;
            offsets[0] = Vector3(0.0f, -diag, diag);
            offsets[1] = Vector3(diag, 0.0f, diag);
            offsets[2] = Vector3(diag, diag, 0.0f);
        }

        int best_axis = -1;
        float best_t = std::numeric_limits<float>::infinity();
        for (int axis = 0; axis < 3; ++axis) {
            const Vector3 center = origin + basis.xform(offsets[axis]);
            float t = 0.0f;
            if (!ray_sphere_intersect(ray_origin, ray_dir, center, handle_pick_radius, t)) continue;
            if (t < best_t) {
                best_t = t;
                best_axis = axis;
            }
        }
        if (best_axis < 0) return false;

        drag_target_index_ = index;
        drag_target_is_character_ = is_character;
        drag_axis_ = best_axis;
        drag_start_pos_ = item->node->get_global_position();
        drag_start_scale_ = item->node->get_scale();
        drag_start_quat_ = item->node->get_basis().get_rotation_quaternion();
        drag_world_axis_ = resolve_drag_world_axis(drag_axis_, *item);
        drag_accumulated_ = 0.0f;
        active_drag_mode_ = (mode == "move") ? DRAG_MOVE : (mode == "scale" ? DRAG_SCALE : DRAG_ROTATE);
        return true;
    }

    bool begin_pose_rotate_drag(const Vector2& screen_pos) {
        Skeleton3D* skel = nullptr;
        int bone_index = -1;
        if (!resolve_selected_bone(skel, bone_index)) return false;

        const Transform3D bone_pose = skel->get_bone_global_pose(bone_index);
        const Vector3 bone_world = skel->to_global(bone_pose.origin);
        const float diag = 0.35f * 0.70710677f;
        const Vector3 offsets[3] = {
            Vector3(0.0f, -diag, diag),
            Vector3(diag, 0.0f, diag),
            Vector3(diag, diag, 0.0f),
        };

        const Vector3 ray_origin = camera_->project_ray_origin(screen_pos);
        const Vector3 ray_dir = camera_->project_ray_normal(screen_pos);
        int best_axis = -1;
        float best_t = std::numeric_limits<float>::infinity();
        for (int axis = 0; axis < 3; ++axis) {
            const Vector3 center = bone_world + offsets[axis];
            float t = 0.0f;
            if (!ray_sphere_intersect(ray_origin, ray_dir, center, 0.14f, t)) continue;
            if (t < best_t) {
                best_t = t;
                best_axis = axis;
            }
        }
        if (best_axis < 0) return false;

        drag_axis_ = best_axis;
        drag_bone_index_ = bone_index;
        drag_start_quat_ = skel->get_bone_pose_rotation(bone_index);
        drag_world_axis_ = axis_base_dir(best_axis);
        drag_accumulated_ = 0.0f;

        const Transform3D bone_global = skel->get_global_transform() * skel->get_bone_global_pose(bone_index);
        const Quaternion bone_world_rot = bone_global.basis.get_rotation_quaternion();
        drag_pre_rot_ = (bone_world_rot * drag_start_quat_.inverse()).normalized();
        active_drag_mode_ = DRAG_POSE_ROTATE;
        return true;
    }

    void update_active_drag(const Vector2&, const Vector2& rel) {
        const float depth_scale = 0.0018f;
        SceneItem* item = drag_target_is_character_ ? at_char(drag_target_index_) : at_prop(drag_target_index_);

        if (active_drag_mode_ == DRAG_MOVE) {
            if (item == nullptr || item->node == nullptr) return;
            const Vector3 base_pos = item->node->get_global_position();
            const Vector2 screen_base = camera_->unproject_position(base_pos);
            const Vector2 screen_end = camera_->unproject_position(base_pos + drag_world_axis_);
            Vector2 screen_axis = screen_end - screen_base;
            const float len = screen_axis.length();
            if (len < 0.01f) return;
            screen_axis /= len;

            const float scalar_move = rel.dot(screen_axis);
            const float depth = camera_->get_global_position().distance_to(base_pos);
            const float world_units_per_px = depth * depth_scale;
            const Vector3 delta = drag_world_axis_ * (scalar_move * world_units_per_px);

            item->node->set_global_position(item->node->get_global_position() + delta);
            sync_item_from_node(*item);
            if (drag_target_is_character_ && drag_target_index_ == selected_character_index_) update_selection_marker();
            if (!drag_target_is_character_ && drag_target_index_ == selected_prop_index_) update_selection_marker();
            emit_selected_object_moved();
            return;
        }

        if (active_drag_mode_ == DRAG_SCALE) {
            if (item == nullptr || item->node == nullptr) return;
            const Vector3 base_pos = item->node->get_global_position();
            const Vector2 screen_base = camera_->unproject_position(base_pos);
            const Vector2 screen_end = camera_->unproject_position(base_pos + drag_world_axis_);
            Vector2 screen_axis = screen_end - screen_base;
            const float len = screen_axis.length();
            if (len < 0.01f) return;
            screen_axis /= len;

            const float scalar_move = rel.dot(screen_axis);
            const float depth = camera_->get_global_position().distance_to(base_pos);
            const float world_units_per_px = depth * depth_scale;
            const float scalar_delta = scalar_move * world_units_per_px;

            Vector3 s = item->node->get_scale();
            if (drag_axis_ == 0) s.x = MAX(0.01f, s.x + scalar_delta);
            else if (drag_axis_ == 1) s.y = MAX(0.01f, s.y + scalar_delta);
            else s.z = MAX(0.01f, s.z + scalar_delta);
            item->node->set_scale(s);

            sync_item_from_node(*item);
            emit_selected_object_moved();
            return;
        }

        if (active_drag_mode_ == DRAG_ROTATE) {
            if (item == nullptr || item->node == nullptr) return;
            const float raw = (drag_axis_ == 0) ? rel.y : (drag_axis_ == 1 ? rel.x : -rel.x);
            drag_accumulated_ += raw * (0.6f * static_cast<float>(Math_PI / 180.0));
            const Quaternion world_delta(drag_world_axis_, drag_accumulated_);
            const Quaternion next_q = (world_delta * drag_start_quat_).normalized();
            item->node->set_rotation(next_q.get_euler());
            sync_item_from_node(*item);
            emit_selected_object_moved();
            return;
        }

        if (active_drag_mode_ == DRAG_POSE_ROTATE) {
            Skeleton3D* skel = nullptr;
            int bone_index = -1;
            if (!resolve_selected_bone(skel, bone_index)) return;
            if (bone_index != drag_bone_index_) return;

            const float raw = (drag_axis_ == 0) ? rel.y : (drag_axis_ == 1 ? rel.x : -rel.x);
            drag_accumulated_ += raw * (0.6f * static_cast<float>(Math_PI / 180.0));
            const Quaternion world_delta(drag_world_axis_, drag_accumulated_);
            Quaternion pose = (drag_pre_rot_.inverse() * world_delta * drag_pre_rot_ * drag_start_quat_).normalized();

            Vector3 e = pose.get_euler();
            e.x = CLAMP(e.x, static_cast<float>(selected_bone_min_x_ * (Math_PI / 180.0)), static_cast<float>(selected_bone_max_x_ * (Math_PI / 180.0)));
            e.y = CLAMP(e.y, static_cast<float>(selected_bone_min_y_ * (Math_PI / 180.0)), static_cast<float>(selected_bone_max_y_ * (Math_PI / 180.0)));
            e.z = CLAMP(e.z, static_cast<float>(selected_bone_min_z_ * (Math_PI / 180.0)), static_cast<float>(selected_bone_max_z_ * (Math_PI / 180.0)));
            pose = Quaternion::from_euler(e).normalized();

            skel->set_bone_pose_rotation(bone_index, pose);
            const String bone_name = skel->get_bone_name(bone_index);
            const String bone_key = build_bone_key(get_active_character_id(), bone_name);
            if (!bone_key.is_empty()) {
                pose_data_[bone_key.utf8().get_data()] = pose;
            }
            update_selected_bone_rotation_cache();
            emit_signal("poseChanged", bone_name);
        }
    }

    void end_active_drag() {
        active_drag_mode_ = DRAG_NONE;
        drag_axis_ = -1;
        drag_target_index_ = -1;
        drag_target_is_character_ = false;
        drag_bone_index_ = -1;
        drag_accumulated_ = 0.0f;
    }

    void pick_at_screen_pos(const Vector2& screen_pos) {
        if (camera_ == nullptr) return;
        const Vector3 ray_origin = camera_->project_ray_origin(screen_pos);
        const Vector3 ray_dir = camera_->project_ray_normal(screen_pos);

        enum class PickKind { None, Character, Prop };
        PickKind best_kind = PickKind::None;
        int best_index = -1;
        float best_t = std::numeric_limits<float>::infinity();

        auto try_pick_item = [&](PickKind kind, int index, const SceneItem& item) {
            if (item.node == nullptr || !item.visible) return;
            float hit_t = std::numeric_limits<float>::infinity();
            if (!raycast_scene_item(item, ray_origin, ray_dir, hit_t)) return;
            if (hit_t < best_t) {
                best_t = hit_t;
                best_kind = kind;
                best_index = index;
            }
        };

        const bool pose_mode = (mode_ == "pose");
        if (pose_mode) {
            if (!selected_bone_name_.is_empty()) {
                int picked_char_index = -1;
                String picked_bone_name;
                if (try_pick_bone_screen(screen_pos, picked_char_index, picked_bone_name)) {
                    if (picked_char_index >= 0) {
                        select_scene_character(picked_char_index);
                    }
                    select_bone_name(picked_bone_name);
                    return;
                }
            }
        }

        if (pose_mode) {
            for (int i = 0; i < static_cast<int>(characters_.size()); ++i) {
                try_pick_item(PickKind::Character, i, characters_[i]);
            }
            // In pose mode we still allow prop selection, but only when no character was hit.
            if (best_kind == PickKind::None) {
                for (int i = 0; i < static_cast<int>(props_.size()); ++i) {
                    try_pick_item(PickKind::Prop, i, props_[i]);
                }
            }
        } else {
            for (int i = 0; i < static_cast<int>(characters_.size()); ++i) {
                try_pick_item(PickKind::Character, i, characters_[i]);
            }
            // Arrange-mode parity: avoid selecting props behind a character mesh.
            if (best_kind == PickKind::None) {
                for (int i = 0; i < static_cast<int>(props_.size()); ++i) {
                    try_pick_item(PickKind::Prop, i, props_[i]);
                }
            }
        }

        // Fallback: if AABB raycast misses (e.g. imported scene graph edge-cases),
        // use screen-space proximity to visible item anchors.
        if (best_kind == PickKind::None) {
            float best_dist = 1.0e9f;
            auto try_pick_screen = [&](PickKind kind, int index, const SceneItem& item, float threshold_px) {
                if (item.node == nullptr || !item.visible) return;
                const Vector3 world = pick_anchor_world(item);
                if (camera_->is_position_behind(world)) return;
                const Vector2 projected = camera_->unproject_position(world);
                const float dist = projected.distance_to(screen_pos);
                if (dist < best_dist && dist <= threshold_px) {
                    best_dist = dist;
                    best_kind = kind;
                    best_index = index;
                }
            };
            for (int i = 0; i < static_cast<int>(characters_.size()); ++i) {
                if (characters_[i].node == nullptr || !characters_[i].visible) continue;
                const Vector3 world = pick_anchor_world(characters_[i]);
                const float cam_dist = camera_->get_global_position().distance_to(world);
                const float dynamic_threshold = CLAMP(4200.0f / MAX(1.0f, cam_dist), 140.0f, 320.0f);
                try_pick_screen(PickKind::Character, i, characters_[i], dynamic_threshold);
            }
            if (best_kind == PickKind::None) {
                for (int i = 0; i < static_cast<int>(props_.size()); ++i) {
                    try_pick_screen(PickKind::Prop, i, props_[i], 40.0f);
                }
            }
        }

        if (best_kind == PickKind::Prop) {
            UtilityFunctions::print(String("[ForgeRunner.Native] pick: prop #") + String::num_int64(best_index));
            select_scene_prop(best_index);
            return;
        }
        if (best_kind == PickKind::Character) {
            UtilityFunctions::print(String("[ForgeRunner.Native] pick: character #") + String::num_int64(best_index));
            select_scene_character(best_index);
            if (auto* ch = at_char(best_index)) {
                if (mode_ == "pose" && ch->node != nullptr) {
                    select_first_bone_from_node(ch->node);
                }
            }
            return;
        }
        UtilityFunctions::print("[ForgeRunner.Native] pick: none");
    }

    static bool intersect_ray_aabb_local(
        const Vector3& origin,
        const Vector3& dir,
        const AABB& box,
        float& out_t)
    {
        const Vector3 bmin = box.position;
        const Vector3 bmax = box.position + box.size;
        float tmin = 0.0f;
        float tmax = std::numeric_limits<float>::infinity();

        auto axis_slab = [&](float o, float d, float mn, float mx) -> bool {
            if (std::abs(d) < 1.0e-6f) {
                return o >= mn && o <= mx;
            }
            const float inv = 1.0f / d;
            float t1 = (mn - o) * inv;
            float t2 = (mx - o) * inv;
            if (t1 > t2) std::swap(t1, t2);
            tmin = MAX(tmin, t1);
            tmax = MIN(tmax, t2);
            return tmax >= tmin;
        };

        if (!axis_slab(origin.x, dir.x, bmin.x, bmax.x)) return false;
        if (!axis_slab(origin.y, dir.y, bmin.y, bmax.y)) return false;
        if (!axis_slab(origin.z, dir.z, bmin.z, bmax.z)) return false;
        if (tmax < 0.0f) return false;
        out_t = tmin >= 0.0f ? tmin : tmax;
        return out_t >= 0.0f;
    }

    static bool raycast_node_meshes(Node* node, const Vector3& ray_origin, const Vector3& ray_dir, float& out_t) {
        bool hit_any = false;
        float best_t = out_t;

        if (auto* mi = Object::cast_to<MeshInstance3D>(node)) {
            if (mi->is_visible()) {
                Ref<Mesh> mesh = mi->get_mesh();
                if (mesh.is_valid()) {
                    const AABB local_aabb = mesh->get_aabb();
                    const Transform3D inv = mi->get_global_transform().affine_inverse();
                    const Vector3 local_origin = inv.xform(ray_origin);
                    const Vector3 local_dir = inv.basis.xform(ray_dir);
                    float t = std::numeric_limits<float>::infinity();
                    if (intersect_ray_aabb_local(local_origin, local_dir, local_aabb, t) && t < best_t) {
                        best_t = t;
                        hit_any = true;
                    }
                }
            }
        }

        const int child_count = node->get_child_count();
        for (int i = 0; i < child_count; ++i) {
            Node* child = node->get_child(i);
            if (child == nullptr) continue;
            float child_t = best_t;
            if (raycast_node_meshes(child, ray_origin, ray_dir, child_t) && child_t < best_t) {
                best_t = child_t;
                hit_any = true;
            }
        }

        if (hit_any) {
            out_t = best_t;
        }
        return hit_any;
    }

    static bool raycast_scene_item(const SceneItem& item, const Vector3& ray_origin, const Vector3& ray_dir, float& out_t) {
        if (item.node == nullptr || !item.visible) return false;
        out_t = std::numeric_limits<float>::infinity();
        return raycast_node_meshes(item.node, ray_origin, ray_dir, out_t);
    }

    static Node3D* find_first_mesh_node(Node* root) {
        if (root == nullptr) return nullptr;
        if (auto* mi = Object::cast_to<MeshInstance3D>(root)) return mi;
        const int child_count = root->get_child_count();
        for (int i = 0; i < child_count; ++i) {
            if (Node3D* found = find_first_mesh_node(root->get_child(i))) return found;
        }
        return nullptr;
    }

    static Vector3 pick_anchor_world(const SceneItem& item) {
        if (item.node == nullptr) return Vector3();
        if (Node3D* mesh_node = find_first_mesh_node(item.node)) {
            return mesh_node->get_global_position();
        }
        return item.node->get_global_position();
    }

    void update_selection_marker() {
        if (selection_marker_ != nullptr) {
            selection_marker_->set_visible(false);
        }
        update_selection_outline();
    }

    void ensure_outline_material() {
        if (outline_material_.is_valid()) return;

        Ref<Shader> shader;
        shader.instantiate();
        shader->set_code(
            "shader_type spatial;\n"
            "render_mode unshaded, cull_front, depth_draw_never;\n"
            "uniform vec4 outline_color : source_color = vec4(1.0, 0.55, 0.05, 1.0);\n"
            "uniform float outline_width = 1.0;\n"
            "void vertex() {\n"
            "    VERTEX += NORMAL * outline_width;\n"
            "}\n"
            "void fragment() {\n"
            "    ALBEDO = outline_color.rgb;\n"
            "    ALPHA = outline_color.a;\n"
            "}\n");

        outline_material_.instantiate();
        outline_material_->set_shader(shader);
        outline_material_->set_shader_parameter("outline_color", Color(1.0f, 0.55f, 0.05f, 1.0f));
        outline_material_->set_shader_parameter("outline_width", 1.0f);
    }

    void collect_mesh_instances(Node* node, std::vector<MeshInstance3D*>& out) {
        if (node == nullptr) return;
        if (auto* mi = Object::cast_to<MeshInstance3D>(node)) {
            if (mi->is_visible()) out.push_back(mi);
        }
        const int child_count = node->get_child_count();
        for (int i = 0; i < child_count; ++i) {
            collect_mesh_instances(node->get_child(i), out);
        }
    }

    void set_outline_for_item(Node3D* node, bool enabled) {
        if (node == nullptr) return;
        std::vector<MeshInstance3D*> meshes;
        collect_mesh_instances(node, meshes);
        for (MeshInstance3D* mesh : meshes) {
            if (mesh == nullptr) continue;
            Ref<Material> overlay;
            if (enabled) {
                overlay = outline_material_;
            }
            mesh->set_material_overlay(overlay);
        }
    }

    void clear_outline_materials() {
        for (SceneItem& item : characters_) {
            set_outline_for_item(item.node, false);
        }
        for (SceneItem& item : props_) {
            set_outline_for_item(item.node, false);
        }
    }

    void update_selection_outline() {
        clear_outline_materials();
        ensure_outline_material();
        if (!outline_material_.is_valid()) return;

        Node3D* selected_node = nullptr;
        if (selected_prop_index_ >= 0) {
            if (SceneItem* p = at_prop(selected_prop_index_)) selected_node = p != nullptr ? p->node : nullptr;
        } else if (selected_character_index_ >= 0) {
            if (SceneItem* c = at_char(selected_character_index_)) selected_node = c != nullptr ? c->node : nullptr;
        }
        if (selected_node == nullptr) return;

        set_outline_for_item(selected_node, true);
    }

    Node3D* load_scene_node(const String& path) {
        if (path.is_empty()) return nullptr;
        const std::string path_utf8 = path.utf8().get_data();
        const String godot_path = path;
        const String path_lower = godot_path.to_lower();
        const bool is_gltf = path_lower.ends_with(".glb") || path_lower.ends_with(".gltf");
        const bool is_res_uri = godot_path.begins_with("res://") || godot_path.begins_with("user://");

        if (path_utf8.rfind("builtin:greybox/", 0) == 0) {
            auto* mesh = memnew(MeshInstance3D);
            Ref<BoxMesh> box;
            box.instantiate();
            if (path_utf8 == "builtin:greybox/tree") {
                box->set_size(Vector3(0.7f, 2.2f, 0.7f));
            } else if (path_utf8 == "builtin:greybox/wall") {
                box->set_size(Vector3(2.5f, 2.0f, 0.3f));
            } else {
                box->set_size(Vector3(1.0f, 1.0f, 1.0f));
            }
            mesh->set_mesh(box);
            Ref<StandardMaterial3D> mat;
            mat.instantiate();
            mat->set_albedo(Color(0.62f, 0.62f, 0.65f, 1.0f));
            mesh->set_material_override(mat);
            return mesh;
        }

        // Avoid noisy "No loader found" logs for absolute .glb/.gltf paths:
        // these should go through GLTFDocument directly.
        if (!is_gltf && is_res_uri && ResourceLoader::get_singleton() != nullptr) {
            Ref<Resource> res = ResourceLoader::get_singleton()->load(path);
            if (res.is_valid()) {
                Ref<PackedScene> ps = res;
                if (ps.is_valid()) {
                    if (Node* inst = ps->instantiate()) {
                        if (auto* n3d = Object::cast_to<Node3D>(inst)) {
                            return n3d;
                        }
                        auto* wrapper = memnew(Node3D);
                        wrapper->add_child(inst);
                        return wrapper;
                    }
                }
            }
        }

        Ref<GLTFDocument> gltf_doc;
        gltf_doc.instantiate();
        Ref<GLTFState> gltf_state;
        gltf_state.instantiate();
        if (gltf_doc.is_valid() && gltf_state.is_valid()) {
            const Error err = gltf_doc->append_from_file(godot_path, gltf_state);
            if (err == OK) {
                if (Node* generated = gltf_doc->generate_scene(gltf_state)) {
                    if (auto* n3d = Object::cast_to<Node3D>(generated)) {
                        return n3d;
                    }
                    auto* wrapper = memnew(Node3D);
                    wrapper->add_child(generated);
                    return wrapper;
                }
            }
        }

        UtilityFunctions::push_warning(String("[ForgeRunner.Native] Could not load 3D scene: ") + path);
        return nullptr;
    }

    Skeleton3D* find_first_skeleton(Node* root) const {
        if (root == nullptr) return nullptr;
        if (auto* skel = Object::cast_to<Skeleton3D>(root)) return skel;
        const int child_count = root->get_child_count();
        for (int i = 0; i < child_count; ++i) {
            Node* child = root->get_child(i);
            if (auto* found = find_first_skeleton(child)) return found;
        }
        return nullptr;
    }

    bool try_pick_bone_screen(const Vector2& screen_pos, int& out_char_index, String& out_bone_name) const {
        if (camera_ == nullptr) return false;

        bool hit = false;
        float best_dist = std::numeric_limits<float>::infinity();
        float best_cam_dist = std::numeric_limits<float>::infinity();
        out_char_index = -1;
        out_bone_name = String();

        for (int char_index = 0; char_index < static_cast<int>(characters_.size()); ++char_index) {
            const SceneItem& item = characters_[char_index];
            if (item.node == nullptr || !item.visible) continue;

            Skeleton3D* skeleton = find_first_skeleton(item.node);
            if (skeleton == nullptr) continue;

            const int bone_count = skeleton->get_bone_count();
            for (int bone_index = 0; bone_index < bone_count; ++bone_index) {
                const Transform3D bone_pose = skeleton->get_bone_global_pose(bone_index);
                const Vector3 bone_world = skeleton->to_global(bone_pose.origin);
                if (camera_->is_position_behind(bone_world)) continue;

                const Vector2 projected = camera_->unproject_position(bone_world);
                const float dist = projected.distance_to(screen_pos);
                const float cam_dist = camera_->get_global_position().distance_to(bone_world);
                const float threshold = CLAMP(2200.0f / MAX(1.0f, cam_dist), 14.0f, 40.0f);
                if (dist > threshold) continue;

                const bool same_dist = std::abs(dist - best_dist) <= 0.001f;
                const bool better_hit = (dist < best_dist) || (same_dist && cam_dist < best_cam_dist);
                if (!better_hit) continue;

                hit = true;
                best_dist = dist;
                best_cam_dist = cam_dist;
                out_char_index = char_index;
                out_bone_name = skeleton->get_bone_name(bone_index);
            }
        }

        return hit;
    }

    void select_bone_name(const String& bone_name) {
        if (bone_name.is_empty()) return;
        if (selected_bone_name_ == bone_name) return;
        selected_bone_name_ = bone_name;
        update_selected_bone_rotation_cache();
        if (!suppress_bone_tree_item_selected_ && !handling_bone_tree_item_selected_) {
            refresh_external_bone_tree();
        }
        emit_signal("boneSelected", selected_bone_name_);
    }

    void select_first_bone_from_node(Node3D* node) {
        if (!auto_select_first_bone_) return;
        if (node == nullptr) return;
        if (Skeleton3D* skel = find_first_skeleton(node)) {
            if (skel->get_bone_count() > 0) {
                select_bone_name(skel->get_bone_name(0));
            }
        }
    }

    static void apply_item_transform(const SceneItem& item) {
        if (item.node == nullptr) return;
        item.node->set_position(Vector3(item.px, item.py, item.pz));
        item.node->set_rotation_degrees(Vector3(item.rx, item.ry, item.rz));
        item.node->set_scale(Vector3(item.sx, item.sy, item.sz));
        item.node->set_visible(item.visible);
    }

    SceneItem* at_char(int index) {
        if (index < 0 || index >= static_cast<int>(characters_.size())) return nullptr;
        return &characters_[index];
    }
    SceneItem* at_prop(int index) {
        if (index < 0 || index >= static_cast<int>(props_.size())) return nullptr;
        return &props_[index];
    }
    static void set_item_pos(SceneItem* item, double x, double y, double z) {
        if (!item) return;
        item->px = static_cast<float>(x); item->py = static_cast<float>(y); item->pz = static_cast<float>(z);
    }
    static void set_item_rot(SceneItem* item, double x, double y, double z) {
        if (!item) return;
        item->rx = static_cast<float>(x); item->ry = static_cast<float>(y); item->rz = static_cast<float>(z);
    }
    static void set_item_scale(SceneItem* item, double x, double y, double z) {
        if (!item) return;
        item->sx = static_cast<float>(x); item->sy = static_cast<float>(y); item->sz = static_cast<float>(z);
    }
    void emit_prop_moved(int index, const SceneItem& item) {
        const String pos_json = String("{\"x\":") + String::num(item.px) + ",\"y\":" + String::num(item.py) + ",\"z\":" + String::num(item.pz) + "}";
        emit_signal("objectMoved", index, pos_json);
    }
    double get_num_char(int index, float SceneItem::* field) const {
        if (index < 0 || index >= static_cast<int>(characters_.size())) return 0.0;
        return characters_[index].*field;
    }
    double get_num_prop(int index, float SceneItem::* field) const {
        if (index < 0 || index >= static_cast<int>(props_.size())) return 0.0;
        return props_[index].*field;
    }

    enum DragMode {
        DRAG_NONE = 0,
        DRAG_MOVE = 1,
        DRAG_SCALE = 2,
        DRAG_ROTATE = 3,
        DRAG_POSE_ROTATE = 4,
    };

    String mode_ = "pose";
    String edit_mode_ = "rotate";
    String transform_space_ = "local";
    String last_project_path_;
    String bone_tree_id_;
    bool joint_spheres_visible_ = true;
    bool frame_export_active_ = false;
    int frame_export_from_ = 0;
    int frame_export_to_ = 0;
    int frame_export_current_ = 0;
    int frame_export_written_ = 0;
    String frame_export_output_dir_;
    String selected_bone_name_;
    double selected_bone_rot_x_ = 0.0;
    double selected_bone_rot_y_ = 0.0;
    double selected_bone_rot_z_ = 0.0;
    double selected_bone_min_x_ = -180.0;
    double selected_bone_min_y_ = -180.0;
    double selected_bone_min_z_ = -180.0;
    double selected_bone_max_x_ = 180.0;
    double selected_bone_max_y_ = 180.0;
    double selected_bone_max_z_ = 180.0;
    String src_;
    bool show_bone_tree_ = false;
    Tree* bone_tree_ = nullptr;
    bool suppress_bone_tree_item_selected_ = false;
    bool handling_bone_tree_item_selected_ = false;
    Ref<ShaderMaterial> outline_material_;
    SubViewport* sub_viewport_ = nullptr;
    Node3D* scene_root_ = nullptr;
    Camera3D* camera_ = nullptr;
    DirectionalLight3D* light_ = nullptr;
    MeshInstance3D* ground_ = nullptr;
    MeshInstance3D* ground_grid_ = nullptr;
    MeshInstance3D* selection_marker_ = nullptr;
    Node3D* gizmo_root_ = nullptr;
    MeshInstance3D* gizmo_axis_shafts_[3] = { nullptr, nullptr, nullptr };
    MeshInstance3D* gizmo_axis_tips_[3] = { nullptr, nullptr, nullptr };
    MeshInstance3D* gizmo_axis_rings_[3] = { nullptr, nullptr, nullptr };
    MeshInstance3D* gizmo_axis_lines_[3] = { nullptr, nullptr, nullptr };
    MeshInstance3D* gizmo_pivot_ = nullptr;
    Ref<StandardMaterial3D> gizmo_axis_materials_[3];
    Ref<SphereMesh> gizmo_sphere_tip_mesh_;
    Ref<BoxMesh> gizmo_box_tip_mesh_;
    Ref<CylinderMesh> gizmo_cone_tip_mesh_;
    Ref<ImmediateMesh> gizmo_diamond_tip_mesh_;
    Vector3 orbit_target_ = Vector3(0.0f, 1.0f, 0.0f);
    float orbit_distance_ = 4.5f;
    float orbit_yaw_ = 0.0f;
    float orbit_pitch_ = 0.22f;
    bool orbit_dragging_ = false;
    bool pan_dragging_ = false;
    bool left_pressed_ = false;
    bool left_moved_ = false;
    Vector2 left_press_pos_ = Vector2();
    Vector2 drag_last_mouse_ = Vector2();
    DragMode active_drag_mode_ = DRAG_NONE;
    int drag_axis_ = -1;
    int drag_target_index_ = -1;
    bool drag_target_is_character_ = false;
    int drag_bone_index_ = -1;
    Vector3 drag_start_pos_ = Vector3();
    Vector3 drag_start_scale_ = Vector3(1.0f, 1.0f, 1.0f);
    Quaternion drag_start_quat_ = Quaternion();
    Quaternion drag_pre_rot_ = Quaternion();
    Vector3 drag_world_axis_ = Vector3(1.0f, 0.0f, 0.0f);
    float drag_accumulated_ = 0.0f;
    int selected_character_index_ = -1;
    int selected_prop_index_ = -1;
    std::map<std::string, Quaternion> pose_data_;
    std::map<std::string, std::string> scene_properties_;
    std::vector<SceneItem> characters_;
    std::vector<SceneItem> props_;
    bool auto_select_first_bone_ = false;
};

// ---------------------------------------------------------------------------
// GDExtension entry points
// ---------------------------------------------------------------------------

void initialize_forge_runner_native(ModuleInitializationLevel p_level) {
    fprintf(stderr, "[FRN] initialize_forge_runner_native level=%d\n", (int)p_level);
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) return;
    ClassDB::register_class<ForgeBgVBoxContainer>();
    ClassDB::register_class<ForgeBgHBoxContainer>();
    ClassDB::register_class<ForgeMarkdownContainer>();
    ClassDB::register_class<ForgeSpeechRecognizerControl>();
    ClassDB::register_class<ForgeWindowDragControl>();
    ClassDB::register_class<ForgeDockingHostControl>();
    ClassDB::register_class<ForgeDockingContainerControl>();
    ClassDB::register_class<ForgeNumberPickerControl>();
    ClassDB::register_class<ForgeTimelineControl>();
    ClassDB::register_class<ForgePosingEditorControl>();
    ClassDB::register_class<ForgeRunnerNativeMain>();
    fprintf(stderr, "[FRN] classes registered\n");
}

void uninitialize_forge_runner_native(ModuleInitializationLevel p_level) {
    (void)p_level;
}

extern "C" {

#if defined(__ANDROID__)
JNIEXPORT void JNICALL Java_io_crowdware_forgestudio_speech_GodotSpeechBridgeNative_callControlMethod(
    JNIEnv* env,
    jclass,
    jlong object_id,
    jstring method_name,
    jstring value_text) {
    if (env == nullptr || object_id <= 0 || method_name == nullptr || value_text == nullptr) {
        return;
    }

    const char* method_utf = env->GetStringUTFChars(method_name, nullptr);
    const char* value_utf = env->GetStringUTFChars(value_text, nullptr);
    if (method_utf == nullptr || value_utf == nullptr) {
        if (method_utf != nullptr) env->ReleaseStringUTFChars(method_name, method_utf);
        if (value_utf != nullptr) env->ReleaseStringUTFChars(value_text, value_utf);
        return;
    }

    const std::string method(method_utf);
    const std::string value(value_utf);
    env->ReleaseStringUTFChars(method_name, method_utf);
    env->ReleaseStringUTFChars(value_text, value_utf);

    // Strict whitelist: do not allow arbitrary method invocation from Java.
    if (method != "_bridgeRawResult" && method != "_bridgeResult" && method != "_bridgeError") {
        return;
    }

    godot::Object* obj = godot::ObjectDB::get_instance(static_cast<uint64_t>(object_id));
    if (obj == nullptr) return;

    obj->call_deferred(godot::StringName(method.c_str()), godot::String::utf8(value.c_str()));
}
#endif

GDExtensionBool GDE_EXPORT forge_runner_native_library_init(
    GDExtensionInterfaceGetProcAddress p_get_proc_address,
    const GDExtensionClassLibraryPtr   p_library,
    GDExtensionInitialization*         r_initialization)
{
    fprintf(stderr, "[FRN] forge_runner_native_library_init called\n");
    GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);
    init_obj.register_initializer(initialize_forge_runner_native);
    init_obj.register_terminator(uninitialize_forge_runner_native);
    init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);
    return init_obj.init();
}
}
