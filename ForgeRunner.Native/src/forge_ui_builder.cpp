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

#include "forge_ui_builder.h"
#include "forge_path_resolver.h"
#include "forge_sms_bridge.h"
#include "generated/schema_properties.h"
#include "generated/schema_types.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <sstream>

// Godot-cpp headers
#include <godot_cpp/classes/aspect_ratio_container.hpp>
#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/center_container.hpp>
#include <godot_cpp/classes/check_box.hpp>
#include <godot_cpp/classes/check_button.hpp>
#include <godot_cpp/classes/code_highlighter.hpp>
#include <godot_cpp/classes/code_edit.hpp>
#include <godot_cpp/classes/color_rect.hpp>
#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/grid_container.hpp>
#include <godot_cpp/classes/h_box_container.hpp>
#include <godot_cpp/classes/h_separator.hpp>
#include <godot_cpp/classes/h_slider.hpp>
#include <godot_cpp/classes/h_split_container.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/item_list.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/line_edit.hpp>
#include <godot_cpp/classes/link_button.hpp>
#include <godot_cpp/classes/margin_container.hpp>
#include <godot_cpp/classes/menu_bar.hpp>
#include <godot_cpp/classes/menu_button.hpp>
#include <godot_cpp/classes/option_button.hpp>
#include <godot_cpp/classes/panel.hpp>
#include <godot_cpp/classes/panel_container.hpp>
#include <godot_cpp/classes/popup_menu.hpp>
#include <godot_cpp/classes/progress_bar.hpp>
#include <godot_cpp/classes/rich_text_label.hpp>
#include <godot_cpp/classes/scroll_container.hpp>
#include <godot_cpp/classes/spin_box.hpp>
#include <godot_cpp/classes/font_file.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/style_box_flat.hpp>
#include <godot_cpp/classes/syntax_highlighter.hpp>
#include <godot_cpp/classes/system_font.hpp>
#include <godot_cpp/classes/tab_bar.hpp>
#include <godot_cpp/classes/tab_container.hpp>
#include <godot_cpp/classes/text_edit.hpp>
#include <godot_cpp/classes/texture_button.hpp>
#include <godot_cpp/classes/texture_rect.hpp>
#include <godot_cpp/classes/tree.hpp>
#include <godot_cpp/classes/v_box_container.hpp>
#include <godot_cpp/classes/v_separator.hpp>
#include <godot_cpp/classes/v_slider.hpp>
#include <godot_cpp/classes/v_split_container.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

// ---------------------------------------------------------------------------
// File-local helpers
// ---------------------------------------------------------------------------

static Ref<ImageTexture> load_texture(const std::string& path) {
    if (path.empty()) return {};
    Ref<Image> img;
    img.instantiate();
    if (img->load(String(path.c_str())) != Error::OK) return {};
    return ImageTexture::create_from_image(img);
}

static std::string to_lower_copy(const std::string& value) {
    std::string out = value;
    std::transform(out.begin(), out.end(), out.begin(),
                   [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
    return out;
}

static std::string trim_copy(const std::string& value) {
    const auto first = std::find_if_not(value.begin(), value.end(), [](unsigned char c){ return std::isspace(c) != 0; });
    if (first == value.end()) return {};
    const auto last = std::find_if_not(value.rbegin(), value.rend(), [](unsigned char c){ return std::isspace(c) != 0; }).base();
    return std::string(first, last);
}

static std::string unquote_copy(const std::string& value) {
    const auto trimmed = trim_copy(value);
    if (trimmed.size() >= 2) {
        const char first = trimmed.front();
        const char last = trimmed.back();
        if ((first == '"' && last == '"') || (first == '\'' && last == '\'')) {
            return trimmed.substr(1, trimmed.size() - 2);
        }
    }
    return trimmed;
}

static void add_keywords(const Ref<CodeHighlighter>& hl, const std::initializer_list<const char*>& tokens, const Color& color) {
    for (const char* token : tokens) {
        hl->add_keyword_color(String(token), color);
    }
}

static Ref<CodeHighlighter> build_sml_highlighter(const Color& keyword_color) {
    Ref<CodeHighlighter> hl;
    hl.instantiate();
    hl->set_number_color(Color(0.97f, 0.82f, 0.58f, 1.0f));
    hl->set_symbol_color(Color(0.83f, 0.86f, 0.90f, 1.0f));
    hl->add_color_region("\"", "\"", Color(0.745f, 0.537f, 0.435f, 1.0f));
    hl->add_color_region("//", "", Color(0.58f, 0.64f, 0.60f, 1.0f), true);
    hl->add_color_region("@", " ", Color(0.80f, 0.90f, 1.0f, 1.0f));
    add_keywords(hl, {"{", "}", ":"}, keyword_color);
    for (const auto& schema_type : kSchemaTypes) {
        hl->add_keyword_color(String(schema_type.name.data()), keyword_color);
    }
    return hl;
}

static Ref<CodeHighlighter> build_sms_highlighter(const Color& keyword_color) {
    Ref<CodeHighlighter> hl;
    hl.instantiate();
    hl->set_number_color(Color(0.97f, 0.82f, 0.58f, 1.0f));
    hl->set_symbol_color(Color(0.83f, 0.86f, 0.90f, 1.0f));
    hl->add_color_region("\"", "\"", Color(0.745f, 0.537f, 0.435f, 1.0f));
    hl->add_color_region("//", "", Color(0.58f, 0.64f, 0.60f, 1.0f), true);
    add_keywords(hl, {
        "fun", "var", "get", "set", "when", "if", "else", "while", "for", "in",
        "break", "continue", "return", "true", "false", "null"
    }, keyword_color);
    return hl;
}

static Ref<CodeHighlighter> build_markdown_highlighter(const Color& keyword_color) {
    Ref<CodeHighlighter> hl;
    hl.instantiate();
    hl->set_symbol_color(Color(0.83f, 0.86f, 0.90f, 1.0f));
    hl->add_color_region("`", "`", Color(0.80f, 0.90f, 1.0f, 1.0f));
    hl->add_color_region("**", "**", Color(0.94f, 0.96f, 0.99f, 1.0f));
    hl->add_color_region("_", "_", Color(0.94f, 0.96f, 0.99f, 1.0f));
    hl->add_color_region("[", ")", Color(0.62f, 0.74f, 0.94f, 1.0f));
    add_keywords(hl, {"#", "##", "###"}, keyword_color);
    return hl;
}

static Ref<CodeHighlighter> build_cs_highlighter(const Color& keyword_color) {
    Ref<CodeHighlighter> hl;
    hl.instantiate();
    hl->set_number_color(Color(0.97f, 0.82f, 0.58f, 1.0f));
    hl->set_symbol_color(Color(0.83f, 0.86f, 0.90f, 1.0f));
    hl->set_function_color(Color(0.94f, 0.96f, 0.99f, 1.0f));
    hl->set_member_variable_color(Color(0.80f, 0.90f, 1.0f, 1.0f));
    hl->add_color_region("\"", "\"", Color(0.745f, 0.537f, 0.435f, 1.0f));
    hl->add_color_region("//", "", Color(0.58f, 0.64f, 0.60f, 1.0f), true);
    hl->add_color_region("/*", "*/", Color(0.58f, 0.64f, 0.60f, 1.0f));
    hl->add_color_region("#", "", Color(0.86f, 0.70f, 1.0f, 1.0f), true);
    add_keywords(hl, {
        "public", "private", "protected", "internal", "class", "interface", "struct", "enum",
        "void", "string", "int", "float", "double", "bool", "var", "new", "return", "if", "else",
        "switch", "case", "for", "foreach", "while", "using", "namespace"
    }, keyword_color);
    return hl;
}

static Ref<CodeHighlighter> create_codeedit_highlighter(const std::string& language, const Color& keyword_color) {
    if (language == "sml") return build_sml_highlighter(keyword_color);
    if (language == "sms") return build_sms_highlighter(keyword_color);
    if (language == "markdown" || language == "md") return build_markdown_highlighter(keyword_color);
    if (language == "cs" || language == "c#") return build_cs_highlighter(keyword_color);
    return {};
}

namespace forge {

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

UiBuilder::UiBuilder(const std::string& base_dir, const std::string& appres_root)
    : base_dir_(base_dir), appres_root_(appres_root) {
    load_strings();
    load_theme();
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

Control* UiBuilder::build(const smlcore::Document& doc, WindowConfig& out_window) {
    forge::SmsBridge::id_map().clear();
    font_deferred_.clear();
    fonts_.clear();

    if (doc.roots.empty()) return memnew(Control);

    const auto& root = doc.roots[0];
    apply_window_props(root, out_window);

    // Extract Fonts block: "FaceName-Weight" or "FaceName" → asset path
    for (const auto& r : doc.roots) {
        std::string rl = r.name;
        std::transform(rl.begin(), rl.end(), rl.begin(),
                       [](unsigned char c){ return std::tolower(c); });
        if (rl == "fonts") {
            for (const auto& prop : r.properties)
                fonts_[prop.name] = prop.value;
        }
    }

    auto* ctrl = build_node(root);
    if (!ctrl) ctrl = memnew(Control);

    // Fill parent viewport
    ctrl->set_anchor_and_offset(SIDE_LEFT,   0.0f, 0.0f);
    ctrl->set_anchor_and_offset(SIDE_TOP,    0.0f, 0.0f);
    ctrl->set_anchor_and_offset(SIDE_RIGHT,  1.0f, 0.0f);
    ctrl->set_anchor_and_offset(SIDE_BOTTOM, 1.0f, 0.0f);

    post_build_pass();

    return ctrl;
}

// ---------------------------------------------------------------------------
// Language detection
// ---------------------------------------------------------------------------

static std::string detect_lang() {
    for (const char* var : {"LANGUAGE", "LC_ALL", "LC_MESSAGES", "LANG"}) {
        const char* v = std::getenv(var);
        if (!v || v[0] == '\0') continue;
        const std::string s(v);
        // "C" / "POSIX" / "C.UTF-8" → skip
        if (s[0] == 'C' && (s.size() == 1 || s[1] == '.' || s[1] == '_')) continue;
        // Extract two-char code: "de_DE.UTF-8" → "de"
        if (s.size() >= 2 && std::isalpha(static_cast<unsigned char>(s[0]))
                          && std::isalpha(static_cast<unsigned char>(s[1]))) {
            char a = static_cast<char>(std::tolower(static_cast<unsigned char>(s[0])));
            char b = static_cast<char>(std::tolower(static_cast<unsigned char>(s[1])));
            std::string lang; lang += a; lang += b;
            return lang;
        }
    }
    return {};
}

// ---------------------------------------------------------------------------
// strings.sml (+ language overlay)
// ---------------------------------------------------------------------------

static void load_strings_from_dir(
    const std::string& dir, const std::string& lang,
    std::unordered_map<std::string, std::string>& out)
{
    auto try_load = [&](const std::string& path) {
        std::ifstream f(path);
        if (!f.is_open()) return;
        std::ostringstream ss; ss << f.rdbuf();
        try {
            auto doc = smlcore::parse_document(ss.str());
            for (const auto& root : doc.roots)
                for (const auto& prop : root.properties)
                    out[prop.name] = prop.value;
        } catch (...) {
            UtilityFunctions::push_warning(String(("[ForgeRunner] Failed to parse " + path).c_str()));
        }
    };

    try_load(dir + "/strings.sml");
    if (!lang.empty() && lang != "en")
        try_load(dir + "/strings-" + lang + ".sml");
}

void UiBuilder::load_strings() {
    const auto lang = detect_lang();
    // Default layer (ForgeRunner built-in), then app layer (wins)
    if (!appres_root_.empty())
        load_strings_from_dir(appres_root_, lang, strings_);
    load_strings_from_dir(base_dir_, lang, strings_);
}

// ---------------------------------------------------------------------------
// theme.sml (Colors / Layouts / Elevations)
// ---------------------------------------------------------------------------

void UiBuilder::load_theme() {
    auto load_one = [&](const std::string& dir) {
        const auto path = dir + "/theme.sml";
        std::ifstream f(path);
        if (!f.is_open()) return;
        std::ostringstream ss; ss << f.rdbuf();
        try {
            auto doc = smlcore::parse_document(ss.str());
            for (const auto& root : doc.roots) {
                std::string rl = root.name;
                std::transform(rl.begin(), rl.end(), rl.begin(),
                               [](unsigned char c){ return std::tolower(c); });

                if (rl == "colors") {
                    for (const auto& prop : root.properties)
                        colors_[prop.name] = prop.value;
                } else if (rl == "layouts") {
                    for (const auto& prop : root.properties)
                        layouts_[prop.name] = prop.value;
                } else if (rl == "elevations") {
                    for (const auto& child : root.children) {
                        auto& ev = elevations_[child.name];
                        ev.clear();
                        for (const auto& prop : child.properties) {
                            // Resolve @Colors.* / @Layouts.* eagerly
                            std::string val = prop.value;
                            if (!val.empty() && val[0] == '@') {
                                const auto dot = val.find('.', 1);
                                if (dot != std::string::npos) {
                                    std::string ns = val.substr(1, dot - 1);
                                    std::transform(ns.begin(), ns.end(), ns.begin(),
                                                   [](unsigned char c){ return std::tolower(c); });
                                    const auto key = val.substr(dot + 1);
                                    if (ns == "colors") {
                                        auto it = colors_.find(key);
                                        if (it != colors_.end()) val = it->second;
                                    } else if (ns == "layouts") {
                                        auto it = layouts_.find(key);
                                        if (it != layouts_.end()) val = it->second;
                                    }
                                }
                            }
                            ev.emplace_back(prop.name, val);
                        }
                    }
                }
            }
        } catch (...) {
            UtilityFunctions::push_warning(String(("[ForgeRunner] Failed to parse " + path).c_str()));
        }
    };

    if (!appres_root_.empty()) load_one(appres_root_);
    load_one(base_dir_);
}

// ---------------------------------------------------------------------------
// Window / splash config
// ---------------------------------------------------------------------------

void UiBuilder::apply_window_props(const smlcore::Node& root, WindowConfig& out) {
    std::string nl = root.name;
    std::transform(nl.begin(), nl.end(), nl.begin(),
                   [](unsigned char c){ return std::tolower(c); });

    out.is_splash = (nl == "splashscreen");

    if (const auto* p = root.find_property("title")) {
        out.title = resolve_ref(*p);
    }
    if (out.title.empty() && !out.is_splash) out.title = "ForgeRunner";

    const auto sz = root.get_value("size");
    if (!sz.empty()) {
        const auto comma = sz.find(',');
        if (comma != std::string::npos) {
            out.width  = parse_int(sz.substr(0, comma), 0);
            out.height = parse_int(sz.substr(comma + 1), 0);
        }
    }
    const auto msz = root.get_value("minSize");
    if (!msz.empty()) {
        const auto comma = msz.find(',');
        if (comma != std::string::npos) {
            out.min_width  = parse_int(msz.substr(0, comma), 0);
            out.min_height = parse_int(msz.substr(comma + 1), 0);
        }
    }
    if (!out.is_splash) {
        // Window node: read SML flags and centre on screen.
        // Splash flags (borderless, always_on_top, extend_to_title) come from
        // project.godot and are reset by forge_runner_main when leaving splash.
        out.extend_to_title  = parse_bool(root.get_value("extendToTitle", "false"), false);
        out.borderless       = parse_bool(root.get_value("borderless",     "false"), false);
        out.center_on_screen = true;
    }

    if (out.is_splash) {
        out.splash_duration_ms   = parse_int(root.get_value("duration", "3000"), 3000);
        out.splash_load_on_ready = root.get_value("loadOnReady");
    }
}

// ---------------------------------------------------------------------------
// Node building - recursive
// ---------------------------------------------------------------------------

// Parse padding tuple "a, b, c, d" or single "a" into left/top/right/bottom.
static void parse_padding(const std::string& v, int& pl, int& pt, int& pr, int& pb) {
    // Try comma-separated: left, top, right, bottom
    std::vector<int> parts;
    std::istringstream iss(v);
    std::string tok;
    while (std::getline(iss, tok, ',')) {
        tok.erase(tok.begin(), std::find_if(tok.begin(), tok.end(), [](unsigned char c){ return !std::isspace(c); }));
        tok.erase(std::find_if(tok.rbegin(), tok.rend(), [](unsigned char c){ return !std::isspace(c); }).base(), tok.end());
        try { parts.push_back(std::stoi(tok)); } catch (...) {}
    }
    if (parts.size() == 4) { pl = parts[0]; pt = parts[1]; pr = parts[2]; pb = parts[3]; }
    else if (parts.size() == 2) { pl = pr = parts[0]; pt = pb = parts[1]; }
    else if (parts.size() == 1) { pl = pt = pr = pb = parts[0]; }
    else { pl = pt = pr = pb = 0; }
}

Control* UiBuilder::build_node(const smlcore::Node& node) {
    std::string nl = node.name;
    std::transform(nl.begin(), nl.end(), nl.begin(),
                   [](unsigned char c){ return std::tolower(c); });

    // If a non-MarginContainer has padding, wrap it so margins work correctly.
    // The wrapper takes layout properties; the inner control handles content.
    const bool is_box = (nl == "vboxcontainer" || nl == "hboxcontainer" ||
                         nl == "panel" || nl == "panelcontainer");
    const bool has_padding = node.has_property("padding") || node.has_property("paddingLeft") ||
                             node.has_property("paddingTop") || node.has_property("paddingRight") ||
                             node.has_property("paddingBottom");

    if (is_box && has_padding) {
        // Create outer MarginContainer for padding + layout properties
        auto* wrapper = memnew(MarginContainer);

        // Parse and apply padding
        int pl = 0, pt = 0, pr = 0, pb = 0;
        if (node.has_property("padding"))
            parse_padding(node.get_value("padding"), pl, pt, pr, pb);
        if (node.has_property("paddingLeft"))   pl = std::stoi(node.get_value("paddingLeft"));
        if (node.has_property("paddingTop"))    pt = std::stoi(node.get_value("paddingTop"));
        if (node.has_property("paddingRight"))  pr = std::stoi(node.get_value("paddingRight"));
        if (node.has_property("paddingBottom")) pb = std::stoi(node.get_value("paddingBottom"));
        wrapper->add_theme_constant_override("margin_left",   pl);
        wrapper->add_theme_constant_override("margin_top",    pt);
        wrapper->add_theme_constant_override("margin_right",  pr);
        wrapper->add_theme_constant_override("margin_bottom", pb);

        // Apply layout/styling properties to the wrapper
        apply_props(wrapper, node);

        // Build the inner container (no children yet, will add below)
        auto* inner = create_control(nl);
        if (!inner) { memdelete(wrapper); return memnew(Control); }

        // Apply content properties (spacing) to inner
        if (node.has_property("spacing")) {
            if (auto* box = Object::cast_to<BoxContainer>(inner))
                box->add_theme_constant_override("separation", std::stoi(node.get_value("spacing")));
        }

        // Inner fills the wrapper
        inner->set_anchor(SIDE_LEFT, 0); inner->set_anchor(SIDE_RIGHT,  1);
        inner->set_anchor(SIDE_TOP,  0); inner->set_anchor(SIDE_BOTTOM, 1);
        inner->set_h_size_flags(Control::SIZE_EXPAND_FILL);
        inner->set_v_size_flags(Control::SIZE_EXPAND_FILL);

        // Build children into inner
        for (const auto& child : node.children) {
            std::string cnl = child.name;
            std::transform(cnl.begin(), cnl.end(), cnl.begin(), [](unsigned char c){ return std::tolower(c); });
            if (cnl == "item") continue;
            auto* cc = build_node(child);
            if (!cc) continue;
            inner->add_child(cc);
        }

        wrapper->add_child(inner);
        return wrapper;
    }

    auto* ctrl = create_control(nl);
    if (!ctrl) return nullptr;

    apply_props(ctrl, node);

    // Special child handling
    if (nl == "menubar") {
        build_menubar_children(ctrl, node);
        return ctrl;
    }

    // PopupMenu items - handled inside build_menubar_children, skip here
    if (nl == "popupmenu") return ctrl;

    // Recurse children
    for (const auto& child : node.children) {
        std::string cnl = child.name;
        std::transform(cnl.begin(), cnl.end(), cnl.begin(),
                       [](unsigned char c){ return std::tolower(c); });
        if (cnl == "item") continue; // menu items handled separately

        auto* child_ctrl = build_node(child);
        if (!child_ctrl) continue;

        // No auto size flags - explicit sizeFlagsHorizontal/Vertical in SML required

        ctrl->add_child(child_ctrl);

        // Set tab title from attached *.title property (DockingContainer / TabContainer)
        if (auto* tc = Object::cast_to<TabContainer>(ctrl)) {
            for (const auto& prop : child.properties) {
                if (prop.name.size() > 6 &&
                    prop.name.substr(prop.name.size() - 6) == ".title") {
                    int idx = tc->get_tab_count() - 1;
                    tc->set_tab_title(idx, String(prop.value.c_str()));
                    break;
                }
            }
        }
    }

    return ctrl;
}

// ---------------------------------------------------------------------------
// Control factory
// ---------------------------------------------------------------------------

Control* UiBuilder::create_control(const std::string& nl) {
    if (nl == "window" || nl == "splashscreen") return memnew(Panel);
    if (nl == "panel")             return memnew(Panel);
    if (nl == "panelcontainer")    return memnew(PanelContainer);
    if (nl == "vboxcontainer") {
        Variant v = ClassDB::instantiate("ForgeBgVBoxContainer");
        if (v.get_type() != Variant::NIL) {
            if (auto* c = Object::cast_to<Control>(static_cast<Object*>(v))) return c;
        }
        return memnew(VBoxContainer);
    }
    if (nl == "hboxcontainer") {
        Variant v = ClassDB::instantiate("ForgeBgHBoxContainer");
        if (v.get_type() != Variant::NIL) {
            if (auto* c = Object::cast_to<Control>(static_cast<Object*>(v))) return c;
        }
        return memnew(HBoxContainer);
    }
    if (nl == "centercontainer")   return memnew(CenterContainer);
    if (nl == "margincontainer")   return memnew(MarginContainer);
    if (nl == "scrollcontainer")   return memnew(ScrollContainer);
    if (nl == "gridcontainer")     return memnew(GridContainer);
    if (nl == "hsplitcontainer")   return memnew(HSplitContainer);
    if (nl == "vsplitcontainer")   return memnew(VSplitContainer);
    if (nl == "aspectratiocontainer") return memnew(AspectRatioContainer);
    if (nl == "label")             return memnew(Label);
    if (nl == "richtextlabel")     return memnew(RichTextLabel);
    if (nl == "button")            return memnew(Button);
    if (nl == "linkbutton")        return memnew(LinkButton);
    if (nl == "checkbox")          return memnew(CheckBox);
    if (nl == "checkbutton")       return memnew(CheckButton);
    if (nl == "lineedit") return memnew(LineEdit);
    if (nl == "numberpicker") {
        Variant v = ClassDB::instantiate("ForgeNumberPickerControl");
        if (v.get_type() != Variant::NIL) {
            if (auto* c = Object::cast_to<Control>(static_cast<Object*>(v))) return c;
        }
        return memnew(LineEdit);
    }
    if (nl == "textedit")          return memnew(TextEdit);
    if (nl == "codeedit")          return memnew(CodeEdit);
    if (nl == "optionbutton")      return memnew(OptionButton);
    if (nl == "menubutton")        return memnew(MenuButton);
    if (nl == "texturebutton")     return memnew(TextureButton);
    if (nl == "texturerect")       return memnew(TextureRect);
    if (nl == "colorrect")         return memnew(ColorRect);
    if (nl == "hseparator")        return memnew(HSeparator);
    if (nl == "vseparator")        return memnew(VSeparator);
    if (nl == "progressbar")       return memnew(ProgressBar);
    if (nl == "hslider" || nl == "slider") return memnew(HSlider);
    if (nl == "vslider")           return memnew(VSlider);
    if (nl == "spinbox")           return memnew(SpinBox);
    if (nl == "tree")              return memnew(Tree);
    if (nl == "itemlist")          return memnew(ItemList);
    if (nl == "tabbar")            return memnew(TabBar);
    if (nl == "tabcontainer")      return memnew(TabContainer);
    if (nl == "menubar")           return memnew(MenuBar);
    if (nl == "popupmenu")         return memnew(Control); // placeholder
    if (nl == "windowdrag") {
        Variant v = ClassDB::instantiate("ForgeWindowDragControl");
        if (v.get_type() != Variant::NIL) {
            Object* obj = v;
            if (auto* c = Object::cast_to<Control>(obj)) return c;
        }
        return memnew(Control);
    }
    if (nl == "speechrecognizer") {
        Variant v = ClassDB::instantiate("ForgeSpeechRecognizerControl");
        if (v.get_type() != Variant::NIL) {
            Object* obj = v;
            if (auto* c = Object::cast_to<Control>(obj)) return c;
        }
        return memnew(Control);
    }
    if (nl == "timeline") {
        Variant v = ClassDB::instantiate("ForgeTimelineControl");
        if (v.get_type() != Variant::NIL) {
            if (auto* c = Object::cast_to<Control>(static_cast<Object*>(v))) return c;
        }
        return memnew(Control);
    }
    if (nl == "posingeditor") {
        Variant v = ClassDB::instantiate("ForgePosingEditorControl");
        if (v.get_type() != Variant::NIL) {
            if (auto* c = Object::cast_to<Control>(static_cast<Object*>(v))) return c;
        }
        return memnew(Control);
    }
    if (nl == "control")           return memnew(Control);

    // Native docking controls (from GDExtension)
    if (nl == "dockinghost") {
        Variant v = ClassDB::instantiate("ForgeDockingHostControl");
        if (v.get_type() != Variant::NIL) {
            Object* obj = v;
            if (auto* c = Object::cast_to<Control>(obj)) return c;
        }
        return memnew(HBoxContainer);
    }
    if (nl == "dockingcontainer") {
        Variant v = ClassDB::instantiate("ForgeDockingContainerControl");
        if (v.get_type() != Variant::NIL) {
            Object* obj = v;
            if (auto* c = Object::cast_to<Control>(obj)) return c;
        }
        return memnew(TabContainer);
    }

    // Markdown container
    if (nl == "markdown") {
        Variant v = ClassDB::instantiate("ForgeMarkdownContainer");
        if (v.get_type() != Variant::NIL) {
            if (auto* c = Object::cast_to<Control>(static_cast<Object*>(v))) return c;
        }
        return memnew(VBoxContainer);
    }

    // Dotted node names (components, ui.NavTab, etc.) → Button placeholder
    if (nl.find('.') != std::string::npos) return memnew(Button);

    // Unknown → generic Control
    return memnew(Control);
}

// ---------------------------------------------------------------------------
// Property mapping
// ---------------------------------------------------------------------------

void UiBuilder::apply_props(Control* ctrl, const smlcore::Node& node) {
    std::string nl = node.name;
    std::transform(nl.begin(), nl.end(), nl.begin(),
                   [](unsigned char c){ return std::tolower(c); });

    // --- id → node name + SMS id_map registration ---
    if (node.has_property("id")) {
        const auto id_val = node.get_value("id");
        ctrl->set_name(String(id_val.c_str()));
        forge::SmsBridge::id_map()[id_val] = ctrl;
    } else {
        for (const auto& prop : node.properties) {
            const auto& pn = prop.name;
            if (pn.size() > 6 && pn.substr(pn.size() - 6) == ".title") {
                ctrl->set_name(String(prop.value.c_str()));
                break;
            }
        }
    }

    // --- visible ---
    if (node.has_property("visible"))
        ctrl->set_visible(parse_bool(node.get_value("visible"), true));

    // --- size: minWidth/minHeight + fixedWidth/fixedHeight/width/height + customMinimumSize ---
    {
        float min_w = ctrl->get_custom_minimum_size().x;
        float min_h = ctrl->get_custom_minimum_size().y;
        bool changed = false;
        if (node.has_property("customMinimumSize")) {
            const auto v = node.get_value("customMinimumSize");
            const auto comma = v.find(',');
            if (comma != std::string::npos) {
                min_w = parse_float(v.substr(0, comma));
                min_h = parse_float(v.substr(comma + 1));
            } else {
                min_w = min_h = parse_float(v);
            }
            changed = true;
        }
        if (node.has_property("minWidth")) {
            min_w = static_cast<float>(parse_int(node.get_value("minWidth")));
            changed = true;
        }
        if (node.has_property("minHeight")) {
            min_h = static_cast<float>(parse_int(node.get_value("minHeight")));
            changed = true;
        }
        for (const char* key : {"fixedWidth", "width"}) {
            if (node.has_property(key)) { min_w = static_cast<float>(parse_int(node.get_value(key))); changed = true; break; }
        }
        for (const char* key : {"fixedHeight", "height"}) {
            if (node.has_property(key)) { min_h = static_cast<float>(parse_int(node.get_value(key))); changed = true; break; }
        }
        if (changed) ctrl->set_custom_minimum_size(Vector2(min_w, min_h));
    }

    // --- anchors (individual floats) ---
    if (node.has_property("anchorLeft"))
        ctrl->set_anchor(SIDE_LEFT,   parse_float(node.get_value("anchorLeft")));
    if (node.has_property("anchorRight"))
        ctrl->set_anchor(SIDE_RIGHT,  parse_float(node.get_value("anchorRight")));
    if (node.has_property("anchorTop"))
        ctrl->set_anchor(SIDE_TOP,    parse_float(node.get_value("anchorTop")));
    if (node.has_property("anchorBottom"))
        ctrl->set_anchor(SIDE_BOTTOM, parse_float(node.get_value("anchorBottom")));

    // --- anchors: left | top | right | bottom (pipe-separated keyword preset) ---
    if (node.has_property("anchors")) {
        const auto av = node.get_value("anchors");
        const bool al = av.find("left")   != std::string::npos;
        const bool ar = av.find("right")  != std::string::npos;
        const bool at = av.find("top")    != std::string::npos;
        const bool ab = av.find("bottom") != std::string::npos;
        if (al || ar || at || ab) {
            ctrl->set_anchor(SIDE_LEFT,   al ? 0.0f : ctrl->get_anchor(SIDE_LEFT));
            ctrl->set_anchor(SIDE_RIGHT,  ar ? 1.0f : ctrl->get_anchor(SIDE_RIGHT));
            ctrl->set_anchor(SIDE_TOP,    at ? 0.0f : ctrl->get_anchor(SIDE_TOP));
            ctrl->set_anchor(SIDE_BOTTOM, ab ? 1.0f : ctrl->get_anchor(SIDE_BOTTOM));
        }
    }

    // --- offsets (explicit sides) ---
    if (node.has_property("offsetLeft"))
        ctrl->set_offset(SIDE_LEFT,   parse_float(node.get_value("offsetLeft")));
    if (node.has_property("offsetTop"))
        ctrl->set_offset(SIDE_TOP,    parse_float(node.get_value("offsetTop")));
    if (node.has_property("offsetRight"))
        ctrl->set_offset(SIDE_RIGHT,  parse_float(node.get_value("offsetRight")));
    if (node.has_property("offsetBottom"))
        ctrl->set_offset(SIDE_BOTTOM, parse_float(node.get_value("offsetBottom")));

    // --- top / left / right / bottom as pixel offsets ---
    if (node.has_property("top"))    ctrl->set_offset(SIDE_TOP,    parse_float(node.get_value("top")));
    if (node.has_property("left"))   ctrl->set_offset(SIDE_LEFT,   parse_float(node.get_value("left")));
    if (node.has_property("right"))  ctrl->set_offset(SIDE_RIGHT,  parse_float(node.get_value("right")));
    if (node.has_property("bottom")) ctrl->set_offset(SIDE_BOTTOM, parse_float(node.get_value("bottom")));

    // --- size flags ---
    if (node.has_property("sizeFlagsHorizontal"))
        ctrl->set_h_size_flags(static_cast<Control::SizeFlags>(
            parse_size_flags(node.get_value("sizeFlagsHorizontal"), Control::SIZE_FILL)));
    if (node.has_property("sizeFlagsVertical"))
        ctrl->set_v_size_flags(static_cast<Control::SizeFlags>(
            parse_size_flags(node.get_value("sizeFlagsVertical"), Control::SIZE_FILL)));

    // --- windowdrag: transparent pass-through ---
    if (nl == "windowdrag")
        ctrl->set_mouse_filter(Control::MOUSE_FILTER_PASS);

    // --- mouse filter ---
    if (node.has_property("mouseFilter")) {
        const auto mf = node.get_value("mouseFilter");
        if (mf == "stop" || mf == "0")   ctrl->set_mouse_filter(Control::MOUSE_FILTER_STOP);
        else if (mf == "pass" || mf == "1") ctrl->set_mouse_filter(Control::MOUSE_FILTER_PASS);
        else if (mf == "ignore" || mf == "2") ctrl->set_mouse_filter(Control::MOUSE_FILTER_IGNORE);
    }

    // --- spacing (BoxContainer) ---
    if (node.has_property("spacing")) {
        if (auto* box = Object::cast_to<BoxContainer>(ctrl))
            box->add_theme_constant_override("separation", parse_int(node.get_value("spacing")));
    }

    // --- padding (MarginContainer) ---
    if (auto* margin = Object::cast_to<MarginContainer>(ctrl)) {
        if (node.has_property("padding")) {
            int v = parse_int(node.get_value("padding"));
            margin->add_theme_constant_override("margin_left",   v);
            margin->add_theme_constant_override("margin_top",    v);
            margin->add_theme_constant_override("margin_right",  v);
            margin->add_theme_constant_override("margin_bottom", v);
        }
        if (node.has_property("paddingLeft"))
            margin->add_theme_constant_override("margin_left",   parse_int(node.get_value("paddingLeft")));
        if (node.has_property("paddingTop"))
            margin->add_theme_constant_override("margin_top",    parse_int(node.get_value("paddingTop")));
        if (node.has_property("paddingRight"))
            margin->add_theme_constant_override("margin_right",  parse_int(node.get_value("paddingRight")));
        if (node.has_property("paddingBottom"))
            margin->add_theme_constant_override("margin_bottom", parse_int(node.get_value("paddingBottom")));
    }

    // --- columns (GridContainer) ---
    if (auto* grid = Object::cast_to<GridContainer>(ctrl)) {
        if (node.has_property("columns"))
            grid->set_columns(parse_int(node.get_value("columns"), 1));
    }

    // --- Label ---
    if (auto* lbl = Object::cast_to<Label>(ctrl)) {
        if (const auto* p = node.find_property("text"))
            lbl->set_text(String(resolve_ref(*p).c_str()));
        if (node.has_property("wrap") && parse_bool(node.get_value("wrap")))
            lbl->set_autowrap_mode(TextServer::AUTOWRAP_WORD_SMART);
        if (node.has_property("align")) {
            const auto a = node.get_value("align");
            if (a == "center") lbl->set_horizontal_alignment(HORIZONTAL_ALIGNMENT_CENTER);
            else if (a == "right") lbl->set_horizontal_alignment(HORIZONTAL_ALIGNMENT_RIGHT);
        }
    }

    // --- RichTextLabel ---
    if (auto* rtl = Object::cast_to<RichTextLabel>(ctrl)) {
        if (const auto* p = node.find_property("text"))
            rtl->set_text(String(resolve_ref(*p).c_str()));
        rtl->set_fit_content(node.has_property("fitContent")
                             ? parse_bool(node.get_value("fitContent")) : false);
    }

    // --- Button / LinkButton / CheckBox / CheckButton ---
    if (auto* btn = Object::cast_to<Button>(ctrl)) {
        if (const auto* p = node.find_property("text"))
            btn->set_text(String(resolve_ref(*p).c_str()));
        if (node.has_property("toggleMode"))
            btn->set_toggle_mode(parse_bool(node.get_value("toggleMode")));
        if (node.has_property("buttonPressed"))
            btn->set_pressed(parse_bool(node.get_value("buttonPressed")));
        if (node.has_property("disabled"))
            btn->set_disabled(parse_bool(node.get_value("disabled")));
        if (node.has_property("flat"))
            btn->set_flat(parse_bool(node.get_value("flat")));
    }

    // --- LinkButton ---
    if (auto* lbtn = Object::cast_to<LinkButton>(ctrl)) {
        if (const auto* p = node.find_property("text"))
            lbtn->set_text(String(resolve_ref(*p).c_str()));
    }

    // --- LineEdit ---
    if (auto* le = Object::cast_to<LineEdit>(ctrl)) {
        if (const auto* p = node.find_property("text"))
            le->set_text(String(resolve_ref(*p).c_str()));
        if (const auto* p = node.find_property("placeholderText"))
            le->set_placeholder(String(resolve_ref(*p).c_str()));
        if (node.has_property("editable"))
            le->set_editable(parse_bool(node.get_value("editable"), true));
    }

    // --- TextEdit ---
    if (auto* te = Object::cast_to<TextEdit>(ctrl)) {
        if (const auto* p = node.find_property("text"))
            te->set_text(String(resolve_ref(*p).c_str()));
    }

    // --- CodeEdit syntax highlighter ---
    if (auto* code_edit = Object::cast_to<CodeEdit>(ctrl)) {
        // Support both legacy "syntax" and new "language" property names.
        std::string language = node.has_property("syntax")
            ? node.get_value("syntax")
            : node.get_value("language", "");
        language = to_lower_copy(unquote_copy(language));
        if (!language.empty()) {
            Color keyword_color(0.357f, 0.533f, 0.769f, 1.0f);
            auto accent_it = colors_.find("accent");
            if (accent_it != colors_.end()) {
                float r = 0.0f, g = 0.0f, b = 0.0f, a = 1.0f;
                if (parse_color(resolve_value(accent_it->second), r, g, b, a)) {
                    keyword_color = Color(r, g, b, a);
                }
            }
            const Ref<CodeHighlighter> hl = create_codeedit_highlighter(language, keyword_color);
            if (hl.is_valid()) {
                code_edit->set_syntax_highlighter(hl);
            } else {
                code_edit->set_syntax_highlighter(Ref<SyntaxHighlighter>());
            }
        }
    }

    // --- TextureRect ---
    if (auto* tr = Object::cast_to<TextureRect>(ctrl)) {
        if (node.has_property("src")) {
            const auto path = resolve_asset_path(node.get_value("src"));
            { auto tex = load_texture(path); if (tex.is_valid()) tr->set_texture(tex); }
        }
        if (node.has_property("shrinkH") || node.has_property("shrinkV")) {
            tr->set_expand_mode(TextureRect::EXPAND_IGNORE_SIZE);
            tr->set_stretch_mode(TextureRect::STRETCH_KEEP_ASPECT_CENTERED);
        }
    }

    // --- TextureButton ---
    if (auto* tb = Object::cast_to<TextureButton>(ctrl)) {
        if (node.has_property("textureNormal")) {
            const auto path = resolve_asset_path(node.get_value("textureNormal"));
            { auto tex = load_texture(path); if (tex.is_valid()) tb->set_texture_normal(tex); }
        }
        if (node.has_property("texturePressed")) {
            const auto path = resolve_asset_path(node.get_value("texturePressed"));
            { auto tex = load_texture(path); if (tex.is_valid()) tb->set_texture_pressed(tex); }
        }
        if (node.has_property("textureHover")) {
            const auto path = resolve_asset_path(node.get_value("textureHover"));
            { auto tex = load_texture(path); if (tex.is_valid()) tb->set_texture_hover(tex); }
        }
        if (node.has_property("toggleMode"))
            tb->set_toggle_mode(parse_bool(node.get_value("toggleMode")));
        if (node.has_property("buttonPressed"))
            tb->set_pressed(parse_bool(node.get_value("buttonPressed")));
        if (node.has_property("ignoreTextureSize"))
            tb->set_ignore_texture_size(parse_bool(node.get_value("ignoreTextureSize")));
        tb->set_stretch_mode(TextureButton::STRETCH_KEEP_ASPECT_CENTERED);
    }

    // --- OptionButton ---
    if (auto* ob = Object::cast_to<OptionButton>(ctrl)) {
        if (node.has_property("fitToLongestItem"))
            ob->set_fit_to_longest_item(parse_bool(node.get_value("fitToLongestItem"), true));
    }

    // --- ProgressBar ---
    if (auto* pb = Object::cast_to<ProgressBar>(ctrl)) {
        if (node.has_property("min"))   pb->set_min(parse_float(node.get_value("min")));
        if (node.has_property("max"))   pb->set_max(parse_float(node.get_value("max")));
        if (node.has_property("value")) pb->set_value(parse_float(node.get_value("value")));
        if (node.has_property("showPercentage"))
            pb->set_show_percentage(parse_bool(node.get_value("showPercentage"), true));
    }

    // --- Tree ---
    if (auto* tree = Object::cast_to<Tree>(ctrl)) {
        if (node.has_property("hideRoot"))
            tree->set_hide_root(parse_bool(node.get_value("hideRoot")));
    }

    // --- TabContainer ---
    if (auto* tc = Object::cast_to<TabContainer>(ctrl)) {
        (void)tc; // children add themselves via add_child; titles are node names
    }

    // --- DockingHost ---
    if (nl == "dockinghost") {
        ctrl->set_h_size_flags(Control::SIZE_EXPAND_FILL);
        ctrl->set_v_size_flags(Control::SIZE_EXPAND_FILL);
        if (node.has_property("gap") && ctrl->has_method("set_gap"))
            ctrl->call("set_gap", parse_float(node.get_value("gap")));
    }

    // --- DockingContainer ---
    if (nl == "dockingcontainer") {
        const auto side = node.get_value("dockSide", "center");
        const bool is_flex = parse_bool(node.get_value("flex", "false"));
        if (ctrl->has_method("set_dock_side"))
            ctrl->call("set_dock_side", String(side.c_str()));
        if (node.has_property("fixedWidth") && ctrl->has_method("set_fixed_width")) {
            if (is_flex) {
                const std::string id = node.get_value("id", "");
                std::string msg = "[ForgeRunner] DockingContainer";
                if (!id.empty()) msg += " id='" + id + "'";
                msg += " has flex:true with fixedWidth. fixedWidth is ignored; use minWidth for constraints.";
                UtilityFunctions::push_warning(String(msg.c_str()));
            } else {
                ctrl->call("set_fixed_width", (double)parse_int(node.get_value("fixedWidth")));
            }
        }
        if (node.has_property("fixedHeight") && ctrl->has_method("set_fixed_height"))
            ctrl->call("set_fixed_height", (double)parse_int(node.get_value("fixedHeight")));
        if (node.has_property("heightPercent") && ctrl->has_method("set_height_percent"))
            ctrl->call("set_height_percent", (double)parse_float(node.get_value("heightPercent")));
        if (ctrl->has_method("set_flex"))
            ctrl->call("set_flex", is_flex);
        if (node.has_property("collapsed") && ctrl->has_method("set_collapsed"))
            ctrl->call("set_collapsed", parse_bool(node.get_value("collapsed"), false));
        if (side == "center") {
            ctrl->set_h_size_flags(Control::SIZE_EXPAND_FILL);
            ctrl->set_v_size_flags(Control::SIZE_EXPAND_FILL);
        } else {
            ctrl->set_h_size_flags(Control::SIZE_FILL);
            ctrl->set_v_size_flags(Control::SIZE_EXPAND_FILL);
        }
        // DockingHost manages column widths. Keep an explicit minWidth/customMinimumSize.x
        // if provided, but do not treat fixedWidth as an implicit minimum.
        float min_x = 0.f;
        if (node.has_property("minWidth")) {
            min_x = static_cast<float>(parse_int(node.get_value("minWidth")));
        } else if (node.has_property("customMinimumSize")) {
            min_x = ctrl->get_custom_minimum_size().x;
        }
        ctrl->set_custom_minimum_size(Vector2(min_x, ctrl->get_custom_minimum_size().y));
        if (auto* tc = Object::cast_to<TabContainer>(ctrl)) {
            if (node.has_property("dragToRearrangeEnabled"))
                tc->set_drag_to_rearrange_enabled(parse_bool(node.get_value("dragToRearrangeEnabled")));
            if (node.has_property("tabsRearrangeGroup"))
                tc->set_tabs_rearrange_group(parse_int(node.get_value("tabsRearrangeGroup")));
        }
    }

    // --- MarkdownContainer ---
    if (nl == "markdown") {
        if (node.has_property("fontSize") && ctrl->has_method("set_base_font_size"))
            ctrl->call("set_base_font_size", (double)parse_float(node.get_value("fontSize")));
        if (node.has_property("bgColor") && ctrl->has_method("set_bg_style")) {
            float r, g, b, a = 1.0f;
            if (parse_color(resolve_value(node.get_value("bgColor")), r, g, b, a)) {
                Ref<StyleBoxFlat> sb;
                sb.instantiate();
                sb->set_bg_color(Color(r, g, b, a));
                ctrl->call("set_bg_style", sb);
            }
        }
        if (node.has_property("src") && ctrl->has_method("set_src"))
            ctrl->call("set_src", String(resolve_asset_path(node.get_value("src")).c_str()));
        else if (node.has_property("text") && ctrl->has_method("set_markdown"))
            ctrl->call("set_markdown", String(resolve_value(node.get_value("text")).c_str()));
    }

    // --- Native PosingEditor ---
    if (nl == "posingeditor") {
        if (node.has_property("src") && ctrl->has_method("set_src")) {
            ctrl->call("set_src", String(resolve_asset_path(node.get_value("src")).c_str()));
        }
        if (node.has_property("showBoneTree") && ctrl->has_method("set_show_bone_tree")) {
            ctrl->call("set_show_bone_tree", parse_bool(node.get_value("showBoneTree"), false));
        }
    }

    // --- Native Timeline ---
    if (nl == "timeline") {
        if (node.has_property("fps") && ctrl->has_method("set_fps")) {
            ctrl->call("set_fps", parse_int(node.get_value("fps"), 24));
        }
        if (node.has_property("totalFrames") && ctrl->has_method("set_total_frames")) {
            ctrl->call("set_total_frames", parse_int(node.get_value("totalFrames"), 120));
        }
    }

    // --- Styling: color (font_color) ---
    if (const auto* p = node.find_property("color")) {
        float r, g, b, a = 1.0f;
        if (parse_color(resolve_ref(*p), r, g, b, a)) {
            Color c(r, g, b, a);
            ctrl->add_theme_color_override("font_color", c);
            ctrl->add_theme_color_override("font_uneditable_color", c);
        }
    }

    // --- Styling: fontSize ---
    if (node.has_property("fontSize"))
        ctrl->add_theme_font_size_override("font_size",
            parse_int(resolve_value(node.get_value("fontSize"))));

    // --- fontFace + fontWeight (deferred - resolved after full tree is built) ---
    if (node.has_property("fontFace") || node.has_property("fontWeight")) {
        FontDeferred d;
        d.ctrl   = ctrl;
        d.face   = node.has_property("fontFace")   ? node.get_value("fontFace")   : "";
        d.weight = node.has_property("fontWeight") ? node.get_value("fontWeight") : "Regular";
        font_deferred_.push_back(d);
    }

    // --- shrinkH / shrinkV (combined with expand when both are present) ---
    const bool has_expand = node.has_property("expand") && parse_bool(node.get_value("expand"));
    if (node.has_property("shrinkH") && !Object::cast_to<TextureRect>(ctrl)) {
        int f = Control::SIZE_SHRINK_CENTER;
        if (has_expand) f |= Control::SIZE_EXPAND;
        ctrl->set_h_size_flags(static_cast<Control::SizeFlags>(f));
    }
    if (node.has_property("shrinkV") && !Object::cast_to<TextureRect>(ctrl)) {
        int f = Control::SIZE_SHRINK_CENTER;
        if (has_expand) f |= Control::SIZE_EXPAND;
        ctrl->set_v_size_flags(static_cast<Control::SizeFlags>(f));
    }
    // expand without shrink: OR SIZE_EXPAND into existing flags
    if (has_expand && !node.has_property("shrinkH") && !node.has_property("shrinkV")) {
        ctrl->set_h_size_flags(static_cast<Control::SizeFlags>(
            ctrl->get_h_size_flags() | Control::SIZE_EXPAND));
        ctrl->set_v_size_flags(static_cast<Control::SizeFlags>(
            ctrl->get_v_size_flags() | Control::SIZE_EXPAND));
    }

    // --- Generic Godot property fallback via schema_properties.h ---
    // For properties not handled by specific cases above, attempt ctrl->set()
    // using the Godot property name from the generated schema.
    {
        std::string type_name = ctrl->get_class().utf8().get_data();
        for (const auto& def : kSchemaProperties) {
            if (def.type_name != type_name) continue;
            const std::string sml_key(def.sml_name);
            if (!node.has_property(sml_key)) continue;
            // Skip properties already handled by specific code above.
            static constexpr const char* kHandled[] = {
                "text", "disabled", "toggleMode", "buttonPressed", "flat",
                "placeholderText", "editable", "wrap", "align", "fitContent",
                "src", "textureNormal", "texturePressed",
                "textureHover", "ignoreTextureSize", "fitToLongestItem",
                "min", "max", "value", "showPercentage", "hideRoot",
                "visible", "id", "spacing", "padding", "paddingLeft",
                "paddingTop", "paddingRight", "paddingBottom", "columns",
                "sizeFlagsHorizontal", "sizeFlagsVertical", "mouseFilter",
                "customMinimumSize", "minWidth", "minHeight", "fixedWidth", "fixedHeight", "width", "height",
                "anchorLeft", "anchorRight", "anchorTop", "anchorBottom", "anchors",
                "offsetLeft", "offsetTop", "offsetRight", "offsetBottom",
                "top", "left", "right", "bottom",
                "color", "fontSize", "fontFace", "fontWeight",
                "expand", "shrinkH", "shrinkV",
                "bgColor", "borderRadius", "borderColor", "borderWidth",
                "borderTop", "borderBottom", "borderLeft", "borderRight", "elevation",
                "shadowColor", "shadowSize", "shadowOffsetX", "shadowOffsetY",
                "highlightColor",
                nullptr
            };
            bool already_handled = false;
            for (int i = 0; kHandled[i] != nullptr; ++i) {
                if (sml_key == kHandled[i]) { already_handled = true; break; }
            }
            if (already_handled) continue;
            const std::string godot_name(def.godot_name);
            ctrl->set(String(godot_name.c_str()), String(node.get_value(sml_key).c_str()));
        }
    }

    // --- Styling: bgColor / border / shadow / elevation ---
    if (node.has_property("bgColor") || node.has_property("borderRadius") ||
        node.has_property("borderColor") || node.has_property("borderWidth") ||
        node.has_property("borderTop") || node.has_property("borderBottom") ||
        node.has_property("borderLeft") || node.has_property("borderRight") ||
        node.has_property("shadowColor") || node.has_property("shadowSize") ||
        node.has_property("shadowOffsetX") || node.has_property("shadowOffsetY") ||
        node.has_property("elevation")) {

        // Collect effective style properties: elevation profile first, node overrides second.
        std::unordered_map<std::string, std::string> sp;

        if (node.has_property("elevation")) {
            const auto ev_name = node.get_value("elevation");
            auto it = elevations_.find(ev_name);
            if (it != elevations_.end()) {
                for (const auto& [k, v] : it->second)
                    sp[k] = v;
            } else {
                UtilityFunctions::push_warning(String(
                    ("[ForgeRunner] elevation '" + ev_name + "' not found").c_str()));
            }
        }

        // Node-level properties override elevation
        for (const char* key : {"bgColor", "borderRadius", "borderColor", "borderWidth",
                                 "borderTop", "borderBottom", "borderLeft", "borderRight",
                                 "shadowColor", "shadowSize", "shadowOffsetX", "shadowOffsetY"}) {
            if (const auto* p = node.find_property(key))
                sp[key] = resolve_ref(*p);
        }

        Ref<StyleBoxFlat> style;
        style.instantiate();

        auto get_sp = [&](const char* k) -> std::string {
            auto it = sp.find(k); return it != sp.end() ? it->second : "";
        };

        if (!get_sp("bgColor").empty()) {
            float r, g, b, a = 1.0f;
            if (parse_color(get_sp("bgColor"), r, g, b, a))
                style->set_bg_color(Color(r, g, b, a));
        }
        if (!get_sp("borderRadius").empty())
            style->set_corner_radius_all(parse_int(get_sp("borderRadius")));
        if (!get_sp("borderColor").empty()) {
            float r, g, b, a = 1.0f;
            if (parse_color(get_sp("borderColor"), r, g, b, a))
                style->set_border_color(Color(r, g, b, a));
        }
        if (!get_sp("borderWidth").empty())
            style->set_border_width_all(parse_int(get_sp("borderWidth")));
        if (!get_sp("borderTop").empty())
            style->set_border_width(SIDE_TOP,    parse_int(get_sp("borderTop")));
        if (!get_sp("borderBottom").empty())
            style->set_border_width(SIDE_BOTTOM, parse_int(get_sp("borderBottom")));
        if (!get_sp("borderLeft").empty())
            style->set_border_width(SIDE_LEFT,   parse_int(get_sp("borderLeft")));
        if (!get_sp("borderRight").empty())
            style->set_border_width(SIDE_RIGHT,  parse_int(get_sp("borderRight")));
        if (!get_sp("shadowSize").empty())
            style->set_shadow_size(parse_int(get_sp("shadowSize")));
        if (!get_sp("shadowColor").empty()) {
            float r, g, b, a = 1.0f;
            if (parse_color(get_sp("shadowColor"), r, g, b, a))
                style->set_shadow_color(Color(r, g, b, a));
        }
        {
            float ox = parse_float(get_sp("shadowOffsetX"));
            float oy = parse_float(get_sp("shadowOffsetY"));
            if (ox != 0.0f || oy != 0.0f)
                style->set_shadow_offset(Vector2(ox, oy));
        }

        // Bg variants (VBox/HBox) use custom _draw(); Panel/PanelContainer use theme slot.
        if (ctrl->has_method("set_bg_style")) {
            ctrl->call("set_bg_style", style);
        } else {
            ctrl->add_theme_stylebox_override("panel", style);
            ctrl->add_theme_stylebox_override("normal", style);
        }
    }

    // --- highlightColor ---
    if (node.has_property("highlightColor") && ctrl->has_method("set_highlight_color")) {
        float r, g, b, a = 1.0f;
        if (parse_color(resolve_value(node.get_value("highlightColor")), r, g, b, a))
            ctrl->call("set_highlight_color", Color(r, g, b, a));
    }
}

// ---------------------------------------------------------------------------
// MenuBar
// ---------------------------------------------------------------------------

void UiBuilder::build_menubar_children(Control* menu_bar, const smlcore::Node& node) {
    auto* mb = Object::cast_to<MenuBar>(menu_bar);
    if (!mb) return;

    if (node.has_property("preferGlobalMenu") && parse_bool(node.get_value("preferGlobalMenu")))
        mb->set_prefer_global_menu(true);

    for (const auto& child : node.children) {
        std::string cnl = child.name;
        std::transform(cnl.begin(), cnl.end(), cnl.begin(),
                       [](unsigned char c){ return std::tolower(c); });
        if (cnl != "popupmenu") continue;

        auto* popup = memnew(PopupMenu);
        const auto popup_id = child.get_value("id", "");
        if (!popup_id.empty()) {
            popup->set_meta("sml_id", String(popup_id.c_str()));
        }
        const auto title = child.get_value("title", child.name);
        popup->set_name(String(title.c_str()));

        int item_id = 1;
        for (const auto& item : child.children) {
            std::string inl = item.name;
            std::transform(inl.begin(), inl.end(), inl.begin(),
                           [](unsigned char c){ return std::tolower(c); });
            if (inl != "item") continue;

            const auto* tp = item.find_property("text");
            std::string text = tp ? resolve_ref(*tp) : "Item";
            popup->add_item(String(text.c_str()), item_id);
            const int idx = popup->get_item_count() - 1;
            const auto item_sml_id = item.get_value("id", "");
            if (!item_sml_id.empty()) {
                popup->set_item_metadata(idx, String(item_sml_id.c_str()));
            }
            item_id++;
        }
        mb->add_child(popup);
    }
}

// ---------------------------------------------------------------------------
// Text / asset resolution
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Post-build pass
// ---------------------------------------------------------------------------

static int weight_name_to_int(const std::string& w) {
    std::string wl = w;
    std::transform(wl.begin(), wl.end(), wl.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    if (wl == "thin")       return 100;
    if (wl == "extralight") return 200;
    if (wl == "light")      return 300;
    if (wl == "regular")    return 400;
    if (wl == "medium")     return 500;
    if (wl == "semibold")   return 600;
    if (wl == "bold")       return 700;
    if (wl == "extrabold")  return 800;
    if (wl == "black")      return 900;
    try { return std::stoi(w); } catch (...) { return 400; }
}

static Ref<Font> try_load_font_file(const std::string& path) {
    if (path.empty()) return {};
    // res:// path → ResourceLoader
    if (path.size() > 6 && path.substr(0, 6) == "res://") {
        Ref<Resource> res = ResourceLoader::get_singleton()->load(String(path.c_str()));
        return Ref<Font>(Object::cast_to<Font>(res.ptr()));
    }
    // Absolute path → FontFile::load_dynamic_font
    Ref<FontFile> f; f.instantiate();
    if (f->load_dynamic_font(String(path.c_str())) == Error::OK) return f;
    return {};
}

void UiBuilder::post_build_pass() {
    for (const auto& d : font_deferred_) {
        if (!d.ctrl) continue;
        const std::string weight = d.weight.empty() ? "Regular" : d.weight;

        if (!d.face.empty()) {
            // Try "FaceName-Weight", then "FaceName" in the Fonts block
            Ref<Font> font;
            for (const auto& key : {d.face + "-" + weight, d.face}) {
                auto it = fonts_.find(key);
                if (it == fonts_.end()) continue;
                const auto path = resolve_asset_path(it->second);
                font = try_load_font_file(path);
                if (font.is_valid()) break;
                UtilityFunctions::push_warning(String(
                    ("[ForgeRunner] Cannot load font file: " + path).c_str()));
            }
            if (font.is_valid()) {
                d.ctrl->add_theme_font_override("font", font);
            } else {
                UtilityFunctions::push_warning(String(
                    ("[ForgeRunner] Font '" + d.face + "-" + weight +
                     "' not found in Fonts block - using system font fallback").c_str()));
                Ref<SystemFont> sf; sf.instantiate();
                sf->set_font_weight(weight_name_to_int(weight));
                d.ctrl->add_theme_font_override("font", sf);
            }
        } else {
            // fontWeight only → SystemFont
            Ref<SystemFont> sf; sf.instantiate();
            sf->set_font_weight(weight_name_to_int(weight));
            d.ctrl->add_theme_font_override("font", sf);
        }
    }
    font_deferred_.clear();
}

std::string UiBuilder::resolve_at_ref(const std::string& ref) const {
    // ref is like "@Colors.accent", "@Layouts.gap", "@Strings.key"
    const auto dot = ref.find('.', 1);
    if (dot == std::string::npos) return {};

    std::string ns = ref.substr(1, dot - 1);
    std::transform(ns.begin(), ns.end(), ns.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    const auto key = ref.substr(dot + 1);

    if (ns == "colors") {
        auto it = colors_.find(key);
        if (it != colors_.end()) return it->second;
        UtilityFunctions::push_warning(String(("[ForgeRunner] @Colors." + key + " not found").c_str()));
        return {};
    }
    if (ns == "layouts") {
        auto it = layouts_.find(key);
        if (it != layouts_.end()) return it->second;
        UtilityFunctions::push_warning(String(("[ForgeRunner] @Layouts." + key + " not found").c_str()));
        return {};
    }
    if (ns == "strings") {
        auto it = strings_.find(key);
        if (it != strings_.end()) return it->second;
        return {};
    }
    return {};
}

std::string UiBuilder::resolve_value(const std::string& v) const {
    if (!v.empty() && v[0] == '@') {
        const auto resolved = resolve_at_ref(v);
        return resolved.empty() ? v : resolved;
    }
    return v;
}

std::string UiBuilder::resolve_ref(const smlcore::Property& prop) const {
    const auto& v = prop.value;

    // Tuple: "@Strings.key, \"Fallback\"" - try the ref first, use fallback if not found
    if (prop.kind == smlcore::ValueKind::Tuple) {
        const auto comma = v.find(',');
        const auto ref   = (comma != std::string::npos) ? v.substr(0, comma) : v;
        std::string key_part = ref;
        key_part.erase(0, key_part.find_first_not_of(" \t\""));
        key_part.erase(key_part.find_last_not_of(" \t\"") + 1);

        std::string resolved;
        if (!key_part.empty() && key_part[0] == '@')
            resolved = resolve_at_ref(key_part);

        if (!resolved.empty()) return resolved;

        // Use fallback (second tuple element, strip quotes)
        if (comma != std::string::npos) {
            std::string fallback = v.substr(comma + 1);
            fallback.erase(0, fallback.find_first_not_of(" \t\""));
            fallback.erase(fallback.find_last_not_of(" \t\"") + 1);
            if (!fallback.empty()) return fallback;
        }
        return key_part;
    }

    // Single reference: @Namespace.key
    if (prop.kind == smlcore::ValueKind::Identifier && !v.empty() && v[0] == '@') {
        const auto resolved = resolve_at_ref(v);
        if (!resolved.empty()) return resolved;
        // Fallback: strip the @Namespace. prefix and return the key
        const auto dot = v.find('.', 1);
        return (dot != std::string::npos) ? v.substr(dot + 1) : v.substr(1);
    }
    return v;
}

std::string UiBuilder::resolve_asset_path(const std::string& raw) const {
    return resolve_runtime_asset_path(raw, base_dir_, appres_root_);
}


// ---------------------------------------------------------------------------
// Parsing helpers
// ---------------------------------------------------------------------------

bool UiBuilder::parse_bool(const std::string& v, bool fallback) {
    if (v == "true" || v == "1")  return true;
    if (v == "false" || v == "0") return false;
    return fallback;
}

int UiBuilder::parse_int(const std::string& v, int fallback) {
    if (v.empty()) return fallback;
    try { return std::stoi(v); } catch (...) { return fallback; }
}

float UiBuilder::parse_float(const std::string& v, float fallback) {
    if (v.empty()) return fallback;
    try { return std::stof(v); } catch (...) { return fallback; }
}

bool UiBuilder::parse_color(const std::string& v, float& r, float& g, float& b, float& a) {
    if (v.empty()) return false;
    // Godot Color::from_string can handle named colors and hex
    Color c = Color::from_string(String(v.c_str()), Color(1, 1, 1, 1));
    r = c.r; g = c.g; b = c.b; a = c.a;
    return true;
}

int UiBuilder::parse_size_flags(const std::string& v, int fallback) {
    // Accept numeric
    if (!v.empty() && (std::isdigit(static_cast<unsigned char>(v[0])) || v[0] == '-')) {
        try { return std::stoi(v); } catch (...) { return fallback; }
    }
    std::string vl = v;
    std::transform(vl.begin(), vl.end(), vl.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    if (vl == "fill")         return Control::SIZE_FILL;
    if (vl == "expand")       return Control::SIZE_EXPAND;
    if (vl == "expandfill")   return Control::SIZE_EXPAND_FILL;
    if (vl == "shrinkcenter") return Control::SIZE_SHRINK_CENTER;
    if (vl == "shrinkbegin")  return Control::SIZE_FILL;
    if (vl == "shrinkend")    return Control::SIZE_SHRINK_END;
    return fallback;
}

} // namespace forge
