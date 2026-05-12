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

#include <sml_document.h>

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <godot_cpp/classes/style_box_flat.hpp>

namespace godot {
class Control;
class Texture2D;
} // namespace godot

namespace forge {

/// Window / splash metadata extracted from the root SML node.
struct WindowConfig {
    std::string title;
    int         width                = 0;
    int         height               = 0;
    int         min_width            = 0;
    int         min_height           = 0;
    bool        extend_to_title      = false;
    bool        borderless           = false;
    bool        center_on_screen     = false;
    bool        is_splash            = false;
    int         splash_duration_ms   = 3000;
    std::string splash_load_on_ready;
};

/// Builds a Godot Control tree from a parsed SML document.
class UiBuilder {
public:
    /// @param base_dir   Absolute directory of the loaded .sml file.
    /// @param appres_root Value of FORGE_RUNNER_APPRES_ROOT (may be empty).
    UiBuilder(const std::string& base_dir, const std::string& appres_root);

    /// Build the UI.  Returns the root Control (caller must add to scene).
    /// Fills @p out_window with window / splash settings from the root node.
    godot::Control* build(const smlcore::Document& doc, WindowConfig& out_window);

    /// Load theme.{mode}.sml (e.g. "dark", "light") and update the color tokens.
    void load_theme_named(const std::string& mode);

    /// Re-apply stored color bindings for one control after a theme switch.
    void apply_color_bindings(const std::string& id, godot::Control* ctrl);

    // Read-only access to current color tokens (for bridge-side re-application).
    const std::unordered_map<std::string, std::string>& colors() const { return colors_; }

    // Stored StyleBoxFlat refs keyed by control id (for in-place bg color updates).
    std::unordered_map<std::string, godot::Ref<godot::StyleBoxFlat>> style_refs;

private:
    std::string base_dir_;
    std::string appres_root_;

    // Design tokens
    std::unordered_map<std::string, std::string> strings_;
    std::unordered_map<std::string, std::string> colors_;
    std::unordered_map<std::string, std::string> layouts_;
    // elevation name → list of (property, resolved_value)
    std::unordered_map<std::string, std::vector<std::pair<std::string, std::string>>> elevations_;
    // "FaceName-Weight" or "FaceName" → asset path (from Fonts {} block in SML)
    std::unordered_map<std::string, std::string> fonts_;


    // Deferred font resolution (fontFace / fontWeight collected during apply_props)
    struct FontDeferred {
        godot::Control* ctrl;
        std::string     face;    // empty if fontWeight-only
        std::string     weight;  // "Regular", "Bold", "700", …
    };
    std::vector<FontDeferred> font_deferred_;

    void load_strings();
    void load_theme();

    /// Apply deferred operations that require the full tree to be built.
    void post_build_pass();

    godot::Control* build_node(const smlcore::Node& node);
    godot::Control* create_control(const std::string& name_lower);
    void            apply_props(godot::Control* ctrl, const smlcore::Node& node);
    void            apply_window_props(const smlcore::Node& root, WindowConfig& out);
    void            build_menubar_children(godot::Control* menu_bar, const smlcore::Node& node);

    /// Resolve a property value that may be a @Namespace.key reference or Tuple.
    std::string resolve_ref(const smlcore::Property& prop) const;
    /// Resolve a raw value string that may be a bare @Namespace.key reference.
    std::string resolve_value(const std::string& v) const;
    /// Resolve a single @Namespace.key reference. Returns "" if not found.
    std::string resolve_at_ref(const std::string& ref) const;

    std::string resolve_asset_path(const std::string& raw) const;

    static bool  parse_bool(const std::string& v, bool fallback = false);
    static int               parse_int(const std::string& v, int fallback = 0);
    static float             parse_float(const std::string& v, float fallback = 0.0f);
    static bool              parse_color(const std::string& v,
                                         float& r, float& g, float& b, float& a);
    static int               parse_size_flags(const std::string& v, int fallback);
};

} // namespace forge
