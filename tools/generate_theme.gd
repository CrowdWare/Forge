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

extends SceneTree

# Reads ForgeRunner/theme.sml and generates ForgeRunner/theme.tres.
# Run via: ./run_runner.sh theme

func _initialize() -> void:
    _generate()
    quit()

func _generate() -> void:
    var colors: Dictionary = {}
    var layouts: Dictionary = {}
    _parse_theme_sml(colors, layouts)

    if colors.is_empty():
        push_error("generate_theme: No colors found in res://theme.sml - aborting.")
        return

    # Layout values with safe defaults
    var btn_pad_h: int = layouts.get("buttonPaddingH", 12)
    var btn_pad_v: int = layouts.get("buttonPaddingV", 7)
    var inp_pad_h: int = layouts.get("inputPaddingH", 8)
    var inp_pad_v: int = layouts.get("inputPaddingV", 6)
    var r: int         = layouts.get("cornerRadius", 6)
    var r_lg: int      = layouts.get("cornerRadiusLarge", 8)
    var tab_pad_h: int = layouts.get("tabPaddingH", 12)
    var tab_pad_v: int = layouts.get("tabPaddingV", 5)
    var font_sz: int   = layouts.get("fontSize", 13)
    var font_sz_tree: int = layouts.get("fontSizeTree", 12)

    # --- StyleBoxFlat instances ---

    # Button: disabled
    var box_btn_disabled := _make_box(
        _color(colors, "bgButtonDisabled", 0.9),
        1,
        _color(colors, "borderMuted", 0.8),
        r, btn_pad_h, btn_pad_v)

    # Focus ring (all controls)
    var box_focus := StyleBoxFlat.new()
    box_focus.draw_center = false
    box_focus.border_width_left   = 2
    box_focus.border_width_top    = 2
    box_focus.border_width_right  = 2
    box_focus.border_width_bottom = 2
    box_focus.border_color = _color(colors, "focus", 0.95)
    box_focus.corner_radius_top_left     = r + 1
    box_focus.corner_radius_top_right    = r + 1
    box_focus.corner_radius_bottom_right = r + 1
    box_focus.corner_radius_bottom_left  = r + 1
    box_focus.content_margin_left   = 2.0
    box_focus.content_margin_top    = 2.0
    box_focus.content_margin_right  = 2.0
    box_focus.content_margin_bottom = 2.0

    var box_btn_hover := _make_box(
        _color(colors, "bgButtonHover"),
        1,
        _color(colors, "borderHover"),
        r, btn_pad_h, btn_pad_v)

    var box_btn_normal := _make_box(
        _color(colors, "bgButtonNormal"),
        1,
        _color(colors, "borderNormal"),
        r, btn_pad_h, btn_pad_v)

    var box_btn_pressed := _make_box(
        _color(colors, "accent"),
        1,
        _color(colors, "accent"),
        r, btn_pad_h, btn_pad_v)

    # Tree / ItemList panel (no visible border)
    var box_tree_panel := _make_box(
        _color(colors, "bgDark"),
        0,
        _color(colors, "borderPanel"),
        r, 6, 6)

    var box_input_normal := _make_box(
        _color(colors, "bgBase"),
        1,
        _color(colors, "borderInput"),
        r, inp_pad_h, inp_pad_v)

    var box_input_focus := _make_box(
        _color(colors, "bgInputFocus"),
        1,
        _color(colors, "accent"),
        r, inp_pad_h, inp_pad_v)

    var box_panel := _make_box(
        _color(colors, "bgMid"),
        1,
        _color(colors, "borderPanel2"),
        r, inp_pad_h, inp_pad_v)

    var box_panel_container := _make_box(
        _color(colors, "bgDark"),
        0,
        _color(colors, "borderPanelContainer"),
        r_lg, 10, 8)

    # Tabs (top corners only)
    var box_tab_hovered := _make_tab_box(
        _color(colors, "bgDark"),
        1,
        _color(colors, "borderTabHover"),
        r, tab_pad_h, tab_pad_v)

    var box_tab_selected := _make_tab_box(
        _color(colors, "accent"),
        1,
        _color(colors, "accent"),
        r, tab_pad_h, tab_pad_v)

    var box_tab_unselected := _make_tab_box(
        _color(colors, "bgDark"),
        1,
        _color(colors, "borderTabUnselected"),
        r, tab_pad_h, tab_pad_v)

    var box_tabbar_bg := StyleBoxFlat.new()
    box_tabbar_bg.bg_color = _color(colors, "bgDark")

    var box_tabcontainer_panel := _make_box(
        _color(colors, "bgDark"),
        1,
        _color(colors, "borderPanel2"),
        r, inp_pad_h, inp_pad_v)

    # TextEdit (no border)
    var box_textedit := StyleBoxFlat.new()
    box_textedit.bg_color = _color(colors, "bgDark")
    box_textedit.corner_radius_top_left     = r
    box_textedit.corner_radius_top_right    = r
    box_textedit.corner_radius_bottom_right = r
    box_textedit.corner_radius_bottom_left  = r
    box_textedit.content_margin_left   = float(inp_pad_h)
    box_textedit.content_margin_top    = float(inp_pad_v)
    box_textedit.content_margin_right  = float(inp_pad_h)
    box_textedit.content_margin_bottom = float(inp_pad_v)

    # --- Build Theme ---
    var theme := Theme.new()

    # Button
    theme.set_color("font_color",          "Button", _color(colors, "textPrimary"))
    theme.set_color("font_disabled_color", "Button", _color(colors, "textDisabled"))
    theme.set_color("font_hover_color",    "Button", _color(colors, "textHover"))
    theme.set_color("font_pressed_color",  "Button", _color(colors, "textPressed"))
    theme.set_font_size("font_size",       "Button", font_sz)
    theme.set_stylebox("disabled", "Button", box_btn_disabled)
    theme.set_stylebox("focus",    "Button", box_focus)
    theme.set_stylebox("hover",    "Button", box_btn_hover)
    theme.set_stylebox("normal",   "Button", box_btn_normal)
    theme.set_stylebox("pressed",  "Button", box_btn_pressed)

    # ScrollBars
    theme.set_color("accent_color", "HScrollBar", _color(colors, "accent"))
    theme.set_color("accent_color", "VScrollBar", _color(colors, "accent"))

    # ItemList
    theme.set_color("font_color",          "ItemList", _color(colors, "textList"))
    theme.set_color("font_selected_color", "ItemList", _color(colors, "textPressed"))
    theme.set_color("selection_color",     "ItemList", _color(colors, "accent"))
    theme.set_stylebox("panel", "ItemList", box_tree_panel)

    # LineEdit
    theme.set_color("caret_color",       "LineEdit", _color(colors, "accent"))
    theme.set_color("font_color",        "LineEdit", _color(colors, "textInput"))
    theme.set_color("placeholder_color", "LineEdit", _color(colors, "textPlaceholder"))
    theme.set_color("selection_color",   "LineEdit", _color(colors, "accent"))
    theme.set_stylebox("empty",     "LineEdit", box_input_normal)
    theme.set_stylebox("focus",     "LineEdit", box_input_focus)
    theme.set_stylebox("normal",    "LineEdit", box_input_normal)
    theme.set_stylebox("read_only", "LineEdit", box_input_normal)

    # Panel / PanelContainer
    theme.set_stylebox("panel", "Panel",          box_panel)
    theme.set_stylebox("panel", "PanelContainer", box_panel_container)

    # TabBar
    theme.set_color("font_disabled_color",  "TabBar", _color(colors, "textTabDisabled"))
    theme.set_color("font_hovered_color",   "TabBar", _color(colors, "textTabHover"))
    theme.set_color("font_selected_color",  "TabBar", _color(colors, "textPressed"))
    theme.set_color("font_unselected_color","TabBar", _color(colors, "textTabUnselected"))
    theme.set_stylebox("tab_hovered",       "TabBar", box_tab_hovered)
    theme.set_stylebox("tab_selected",      "TabBar", box_tab_selected)
    theme.set_stylebox("tab_unselected",    "TabBar", box_tab_unselected)
    theme.set_stylebox("tabbar_background", "TabBar", box_tabbar_bg)

    # TabContainer
    theme.set_stylebox("panel",             "TabContainer", box_tabcontainer_panel)
    theme.set_stylebox("tab_hovered",       "TabContainer", box_tab_hovered)
    theme.set_stylebox("tab_selected",      "TabContainer", box_tab_selected)
    theme.set_stylebox("tab_unselected",    "TabContainer", box_tab_unselected)
    theme.set_stylebox("tabbar_background", "TabContainer", box_tabbar_bg)

    # TextEdit
    theme.set_color("caret_color",     "TextEdit", _color(colors, "accent"))
    theme.set_color("font_color",      "TextEdit", _color(colors, "textInput"))
    theme.set_color("selection_color", "TextEdit", _color(colors, "selectionGreen", 0.55))
    theme.set_stylebox("focus",     "TextEdit", box_textedit)
    theme.set_stylebox("normal",    "TextEdit", box_textedit)
    theme.set_stylebox("read_only", "TextEdit", box_textedit)

    # Tree
    theme.set_color("font_color",              "Tree", _color(colors, "textList"))
    theme.set_color("font_selected_color",     "Tree", _color(colors, "textPressed"))
    theme.set_color("guide_color",             "Tree", _color(colors, "borderTreeGuide", 0.9))
    theme.set_color("relationship_line_color", "Tree", _color(colors, "borderTreeGuide", 0.9))
    theme.set_color("selection_color",         "Tree", _color(colors, "accent"))
    theme.set_font_size("font_size",           "Tree", font_sz_tree)
    theme.set_stylebox("panel",                "Tree", box_tree_panel)

    # DockingContainerControl (custom node)
    theme.set_color("dock_background", "DockingContainerControl", _color(colors, "bgPanel"))

    # --- Save ---
    var err := ResourceSaver.save(theme, "res://theme.tres")
    if err != OK:
        push_error("generate_theme: Failed to save theme.tres (error %d)" % err)
        return

    _insert_header("res://theme.tres")
    print("Theme generated: res://theme.tres")


# --- Helpers ---

func _color(colors: Dictionary, key: String, alpha: float = 1.0) -> Color:
    var hex: String = colors.get(key, "#FF00FF")  # magenta signals a missing token
    var c := Color(hex)
    c.a = alpha
    return c


func _make_box(bg: Color, border_w: int, border_c: Color, radius: int, pad_h: int, pad_v: int) -> StyleBoxFlat:
    var box := StyleBoxFlat.new()
    box.bg_color = bg
    box.border_width_left   = border_w
    box.border_width_top    = border_w
    box.border_width_right  = border_w
    box.border_width_bottom = border_w
    box.border_color = border_c
    box.corner_radius_top_left     = radius
    box.corner_radius_top_right    = radius
    box.corner_radius_bottom_right = radius
    box.corner_radius_bottom_left  = radius
    box.content_margin_left   = float(pad_h)
    box.content_margin_top    = float(pad_v)
    box.content_margin_right  = float(pad_h)
    box.content_margin_bottom = float(pad_v)
    return box


func _make_tab_box(bg: Color, border_w: int, border_c: Color, radius: int, pad_h: int, pad_v: int) -> StyleBoxFlat:
    var box := StyleBoxFlat.new()
    box.bg_color = bg
    box.border_width_left   = border_w
    box.border_width_top    = border_w
    box.border_width_right  = border_w
    box.border_width_bottom = border_w
    box.border_color = border_c
    box.corner_radius_top_left  = radius
    box.corner_radius_top_right = radius
    # bottom corners stay 0 for tab shapes
    box.content_margin_left   = float(pad_h)
    box.content_margin_top    = float(pad_v)
    box.content_margin_right  = float(pad_h)
    box.content_margin_bottom = float(pad_v)
    return box


func _parse_theme_sml(colors: Dictionary, layouts: Dictionary) -> void:
    var path := "res://theme.sml"
    var file := FileAccess.open(path, FileAccess.READ)
    if file == null:
        push_error("generate_theme: Cannot open " + path)
        return

    var current_block := ""
    while not file.eof_reached():
        var line: String = file.get_line().strip_edges()

        if line.is_empty() or line.begins_with("//"):
            continue

        if line.begins_with("Colors") and line.ends_with("{"):
            current_block = "Colors"
            continue
        if line.begins_with("Layouts") and line.ends_with("{"):
            current_block = "Layouts"
            continue
        if line == "}":
            current_block = ""
            continue
        if current_block.is_empty():
            continue

        var colon_idx := line.find(":")
        if colon_idx < 1:
            continue

        var key: String = line.left(colon_idx).strip_edges()
        var raw: String = line.substr(colon_idx + 1).strip_edges()

        # Strip inline comment
        var ci := raw.find("//")
        if ci != -1:
            raw = raw.left(ci).strip_edges()
        if raw.is_empty():
            continue

        if current_block == "Colors":
            if raw.begins_with("\"") and raw.ends_with("\""):
                colors[key] = raw.substr(1, raw.length() - 2)
        elif current_block == "Layouts":
            if raw.is_valid_int():
                layouts[key] = raw.to_int()

    file.close()


func _insert_header(res_path: String) -> void:
    var abs_path := ProjectSettings.globalize_path(res_path)
    var original := FileAccess.get_file_as_string(abs_path)
    if original.is_empty():
        return

    var newline := "\n"
    var first_newline := original.find(newline)
    if first_newline == -1:
        return

    var first_line := original.left(first_newline)
    var rest      := original.substr(first_newline)

    var header := (
        newline +
        "; ==============================================================" + newline +
        "; AUTO-GENERATED - DO NOT EDIT THIS FILE" + newline +
        "; Source: ForgeRunner/theme.sml" + newline +
        "; Regenerate: ./run_runner.sh theme" + newline +
        "; Reference: THEME.md" + newline +
        "; ==============================================================" + newline
    )

    var file := FileAccess.open(abs_path, FileAccess.WRITE)
    if file == null:
        push_error("generate_theme: Cannot write header to " + abs_path)
        return
    file.store_string(first_line + header + rest)
    file.close()
