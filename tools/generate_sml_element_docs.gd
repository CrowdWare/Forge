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

# Set to true once ForgeRunner.Native/Generated supports .cpp code generation.
const GENERATE_CODE := false

var REPO_ROOT := ""
var SPECS: Dictionary = {}
var OUT_DIR := ""
var SPEC_DIR := ""
var GENERATED_DIR := ""
var NATIVE_GENERATED_DIR := ""
func _initialize() -> void:
    REPO_ROOT = ProjectSettings.globalize_path("res://") + "/../.."
    OUT_DIR = REPO_ROOT + "/docs/SML/Elements"
    SPEC_DIR = REPO_ROOT + "/tools/specs"
    GENERATED_DIR = REPO_ROOT + "/ForgeRunner.Native/Generated"
    NATIVE_GENERATED_DIR = REPO_ROOT + "/ForgeRunner.Native/src/generated"
    _run()
    quit()

const EXTRA_CLASSES := [
    "Window",
    "AcceptDialog",
    "ConfirmationDialog",
    "FileDialog",
    "Popup",
    "PopupMenu",
    "PopupPanel",
    "SubViewport"
]

# CHANGED: List of base-only classes for inheritance docs, not SML elements.
const BASE_ONLY_CLASSES := [
    "Object",
    "Node",
    "CanvasItem",
    "Viewport",
    "OpenXRInteractionProfileEditorBase",
    "BaseButton",
    "Range",
    "ScrollBar",
    "Slider",
    "Separator"
]  # CHANGED

func _run() -> void:
    SPECS = _load_specs()
    var specs := SPECS

    var manual_types: Array[String] = []
    for k in specs.keys():
        var spec_name := String(k)
        if _is_non_element_spec(spec_name):
            continue
        manual_types.append(spec_name)
    manual_types.sort()

    var manual_backing := {}
    for mt in manual_types:
        manual_backing[mt] = String(specs[mt].get("backing", "Control"))

    _ensure_out_dir()
    # Collect SML-instantiable UI classes (Controls + explicit extras)
    # and separately collect all doc targets including required base classes.
    var sml_targets := {}
    var doc_targets := {}

    var classes := ClassDB.get_class_list()
    classes.sort()

    # Controls are SML-instantiable
    for c in classes:
        var cn := String(c)
        if _is_control(cn) and _is_runtime_control_candidate(cn):
            sml_targets[cn] = true
            var cur: String = cn
            while cur != "":
                doc_targets[cur] = true
                cur = String(ClassDB.get_parent_class(cur))

    # Extra UI classes are also SML-instantiable (even if not Control)
    for cn in EXTRA_CLASSES:
        if ClassDB.class_exists(cn):
            sml_targets[cn] = true
            var cur: String = cn
            while cur != "":
                doc_targets[cur] = true
                cur = String(ClassDB.get_parent_class(cur))

    # Manual/spec SML elements are SML-instantiable; their inheritance chain is handled in _inheritance_chain
    for mt in manual_types:
        sml_targets[mt] = true

    # Generate docs for all doc targets + all manual types
    var doc_names: Array = doc_targets.keys()
    for mt in manual_types:
        if not doc_names.has(mt):
            doc_names.append(mt)
    doc_names.sort()

    for name in doc_names:
        _generate_doc(String(name))

    # Generate SML reference only for SML-instantiable types (exclude base-only classes like Node/CanvasItem/Object)
    var sml_names: Array = sml_targets.keys()
    sml_names.sort()
    if GENERATE_CODE:
        _generate_csharp_schema_files(sml_names)

    _generate_native_schema_files(sml_names)

    print("SML element docs generated.")

func _ensure_out_dir() -> void:
    DirAccess.make_dir_recursive_absolute(OUT_DIR)
    if GENERATE_CODE:
        DirAccess.make_dir_recursive_absolute(GENERATED_DIR)
    DirAccess.make_dir_recursive_absolute(NATIVE_GENERATED_DIR)

func _is_control(c_name: String) -> bool:
    return c_name == "Control" or ClassDB.is_parent_class(c_name, "Control")


func _is_runtime_control_candidate(c_name: String) -> bool:
    if c_name.begins_with("Editor"):
        return false
    if c_name.begins_with("OpenXR"):
        return false
    return true

func _generate_doc(c_name: String) -> void:
    var inheritance := _inheritance_chain(c_name)
    var props := _collect_properties(c_name)
    var signals := _collect_signals(c_name)

    var md := "# %s\n\n" % c_name
    if c_name in BASE_ONLY_CLASSES:  # CHANGED
        md += "> Note: This is a base class included for inheritance documentation. It is **not** an SML element.\n\n"  # CHANGED
    md += "## Inheritance\n\n"
    # Create a linked inheritance chain: Class → Parent → ... → Object
    var linked: Array[String] = []
    for cls in inheritance:
        linked.append("[%s](%s.md)" % [cls, cls])
    md += " → ".join(linked) + "\n\n"

    if c_name != "PopupMenu" and _is_collection_control(c_name):  # CHANGED
        md += "## Collection Items\n\n"  # CHANGED
        md += "This control appears to manage internal **items** (collection-style API).\n"  # CHANGED
        md += "Items are typically not represented as child nodes/properties in Godot.\n"  # CHANGED
        md += "In SML, this will be represented via **pseudo child elements** (documented per control).\n\n"  # CHANGED

    # Back-navigation: for base classes, list derived classes.
    if c_name == "Control":
        md += "## Derived Controls\n\n"
        md += "This is the entry point for all UI controls.\n"
        md += "All classes listed below inherit from `Control`.\n\n"

        md += "### Direct subclasses\n\n"
        #md += _md_link_list(_collect_subclasses("Control", true))
        md += _md_link_list(_collect_subclasses_with_manual("Control", true))

        md += "\n### All descendants (alphabetical)\n\n"
        #md += _md_link_list(_collect_subclasses("Control", false))
        md += _md_link_list(_collect_subclasses_with_manual("Control", false)) 

        md += "\n"

    elif c_name in ["CanvasItem", "Node", "Object"]:
        md += "## Derived Classes\n\n"
        md += "Classes listed below inherit from `" + c_name + "`.\n\n"
        md += "### Direct subclasses\n\n"
        #md += _md_link_list(_collect_subclasses(c_name, true))
        md += _md_link_list(_collect_subclasses_with_manual(c_name, true))
        md += "\n"

    else:
        #var direct := _collect_subclasses(c_name, true)
        var direct := _collect_subclasses_with_manual(c_name, true)
        if direct.size() > 0:
            md += "## Derived Classes\n\n"
            md += "### Direct subclasses\n\n"
            md += _md_link_list(direct)
            md += "\n"

    md += "## Properties\n\n"
    md += "This page lists **only properties declared by `" + c_name + "`**.\n"
    var parent := ""  # CHANGED
    if ClassDB.class_exists(c_name):  # CHANGED
        parent = String(ClassDB.get_parent_class(c_name))  # CHANGED
    elif inheritance.size() > 1:  # CHANGED
        parent = inheritance[1]  # CHANGED
    if parent != "":
        md += "Inherited properties are documented in: [" + parent + "](" + parent + ".md)\n\n"
    else:
        md += "\n"

    md += "| Godot Property | SML Property | Type | Default |\n|-|-|-|-|\n"  # CHANGED
    # CHANGED: Manual SML-only element (not a Godot class)
    var spec_props := _get_spec_props(c_name)  # CHANGED
    var override_props := _get_runtime_override_props(c_name)
    var merged_spec_props := []
    merged_spec_props.append_array(spec_props)
    merged_spec_props.append_array(override_props)

    if merged_spec_props.size() > 0:  # CHANGED
        # Spec-defined properties (SML-level, not necessarily Godot properties)
        for sp in merged_spec_props:
            var godot_p := String(sp.get("godot", "-"))  # CHANGED (optional)
            var sml_p := String(sp.get("sml", ""))  # CHANGED
            var typ := String(sp.get("type", "Variant"))  # CHANGED
            var defv := String(sp.get("default", "-"))  # CHANGED
            md += "| %s | %s | %s | %s |\n" % [godot_p, sml_p, typ, defv]  # CHANGED
    else:
        for p in props:
            md += "| %s | %s | %s | - |\n" % [String(p["name"]), _normalize_property(String(p["name"])), String(p["type"])]  # CHANGED


    if SPECS.has(c_name):  # CHANGED
        var notes := Array(SPECS[c_name].get("notes", []))
        if notes.size() > 0:
            md += "\n"
            for n in notes:
                md += "> " + String(n) + "\n"
            md += "\n"

        var ex_sml := Array(SPECS[c_name].get("examples_sml", []))
        if ex_sml.size() > 0:
            md += "### Examples\n\n```sml\n"
            for l in ex_sml:
                md += String(l) + "\n"
            md += "```\n"


    md += "\n## Events\n\n"
    md += "This page lists **only signals declared by `" + c_name + "`**.\n"
    if parent != "":
        md += "Inherited signals are documented in: [" + parent + "](" + parent + ".md)\n\n"
    else:
        md += "\n"

    md += "| Godot Signal | SMS Event | Params |\n|-|-|-|\n"  # unchanged header (kept)
    for s in signals:
        md += "| %s | %s | %s |\n" % [String(s["name"]), _format_sms_handler(String(s["name"]), String(s["params_names"])), String(s["params"]) ]  # CHANGED


    var godotActions := _collect_actions(c_name)
    if godotActions.size() > 0:
        md += "\n## Runtime Actions\n\n"
        md += "This page lists **callable methods declared by `" + c_name + "`**.\n"
        if parent != "":
            md += "Inherited actions are documented in: [" + parent + "](" + parent + ".md)\n\n"
        else:
            md += "\n"

        md += "| Godot Method | SMS Call | Params | Returns |\n|-|-|-|-|\n"

        for a in godotActions:
            var sms: String = String(a["sms"])  # CHANGED
            var pn: String = String(a["params_names"])  # CHANGED
            var call: String = "`<id>." + sms + "(" + pn + ")`"  # CHANGED
            if pn == "":  # CHANGED
                call = "`<id>." + sms + "()`"  # CHANGED

            md += "| %s | %s | %s | %s |\n" % [String(a["name"]), call, String(a["params"]), String(a["returns"])]

    # Actions (from specs)  # CHANGED
    var actions := _get_spec_actions(c_name)  # CHANGED
    var runtime_override_actions := _get_runtime_override_actions(c_name)
    var merged_actions := []
    merged_actions.append_array(actions)
    merged_actions.append_array(runtime_override_actions)
    if merged_actions.size() > 0:  # CHANGED
        md += "\n## Actions\n\n"  # CHANGED
        md += "This page lists **only actions supported by the runtime** for `" + c_name + "`.\n"  # CHANGED
        if parent != "":  # CHANGED
            md += "Inherited actions are documented in: [" + parent + "](" + parent + ".md)\n\n"  # CHANGED
        else:  # CHANGED
            md += "\n"  # CHANGED

        md += "| Action | SMS Call | Params | Returns |\n|-|-|-|-|\n"  # CHANGED

        for a in merged_actions:  # CHANGED
            var an := String(a.get("sms", ""))  # CHANGED
            var params := Array(a.get("params", []))  # CHANGED
            var ret := String(a.get("returns", "void"))  # CHANGED

            var param_sig := "-"  # CHANGED
            var arg_names: Array[String] = []  # CHANGED
            if params.size() > 0:  # CHANGED
                var parts: Array[String] = []  # CHANGED
                for p in params:  # CHANGED
                    var pn := String(p.get("name", "arg"))  # CHANGED
                    var pt := String(p.get("type", "Variant"))  # CHANGED
                    parts.append(pt + " " + pn)  # CHANGED
                    arg_names.append(pn)  # CHANGED
                param_sig = ", ".join(parts)  # CHANGED

            var call := "`<id>." + an + "()`"  # CHANGED
            if arg_names.size() > 0:  # CHANGED
                call = "`<id>." + an + "(" + ", ".join(arg_names) + ")`"  # CHANGED

            md += "| %s | %s | %s | %s |\n" % [an, call, param_sig, ret]  # CHANGED

    # SML-only child structure for menu items (PopupMenu items are not properties in Godot).
    if c_name == "PopupMenu":
        md += "\n## SML Items\n\n"
        md += "`PopupMenu` items are defined as **SML child nodes** (pseudo elements).\n"
        md += "The runtime converts them to Godot menu items internally.\n\n"

        md += "### Supported item elements\n\n"
        md += "- `Item`\n"
        md += "- `CheckItem`\n"
        md += "- `Separator`\n\n"

        md += "### Item properties (SML)\n\n"
        md += "| Property | Type | Default | Notes |\n|-|-|-|-|\n"
        md += "| id | identifier | - | Optional. Enables id-based event sugar (`on <id>.clicked() { ... }`). |\n"
        md += "| text | string | \"\" | Display text. |\n"
        md += "| checked | bool | false | Only for `CheckItem`. |\n"
        md += "| disabled | bool | false | Optional. |\n"

        md += "\n### Example\n\n"
        md += "```sml\n"
        md += "PopupMenu { id: fileMenu\n"
        md += "    Item { id: open; text: \"Open\" }\n"
        md += "    Item { id: save; text: \"Save\" }\n"
        md += "    Separator { }\n"
        md += "    CheckItem { id: autosave; text: \"Auto Save\"; checked: true }\n"
        md += "}\n"
        md += "```\n\n"

        md += "### SMS Event Examples\n\n"
        md += "```sms\n"
        md += "// With explicit item ids:\n"
        md += "on open.clicked() { ... }\n"
        md += "on autosave.clicked() { ... }\n\n"
        md += "// Without item ids (container fallback):\n"
        md += "on fileMenu.idPressed(id) { ... }\n"
        md += "```\n"

    # SML-only child structure for ItemList items (ItemList manages items internally in Godot).
    if c_name == "ItemList":
        md += "\n## SML Items\n\n"
        md += "`ItemList` entries are defined as **SML child nodes** (pseudo elements).\n"
        md += "The runtime converts them to Godot ItemList items internally.\n\n"

        md += "### Supported item elements\n\n"
        md += "- `Item`\n\n"

        md += "### Item properties (SML)\n\n"
        md += "| Property | Type | Default | Notes |\n|-|-|-|-|\n"
        md += "| id | identifier | - | Optional. Enables id-based event sugar (`on <id>.selected() { ... }`). |\n"
        md += "| text | string | \"\" | Display text. |\n"
        md += "| icon | string | \"\" | Optional icon resource/path. |\n"
        md += "| selected | bool | false | Initial selection state (single-select). |\n"
        md += "| disabled | bool | false | Disables the item. |\n"
        md += "| tooltip | string | \"\" | Optional tooltip text. |\n"

        md += "\n### Example\n\n"
        md += "```sml\n"
        md += "ItemList { id: files\n"
        md += "    Item { id: a; text: \"Readme.md\"; icon: \"res:/icons/doc.svg\" }\n"
        md += "    Item { id: b; text: \"Todo.md\" }\n"
        md += "    Item { text: \"Disabled item\"; disabled: true }\n"
        md += "}\n"
        md += "```\n\n"

        md += "### SMS Event Examples\n\n"
        md += "```sms\n"
        md += "// With explicit item ids:\n"
        md += "on a.selected() { ... }\n\n"
        md += "// Without item ids (container fallback):\n"
        md += "on files.itemSelected(index) { ... }\n"
        md += "```\n"

    # SML-only child structure for OptionButton items.
    if c_name == "OptionButton":
        md += "\n## SML Items\n\n"
        md += "`OptionButton` options are defined as **SML child nodes** (pseudo elements).\n"
        md += "The runtime converts them to Godot OptionButton items internally.\n\n"

        md += "### Supported item elements\n\n"
        md += "- `Item`\n\n"

        md += "### Item properties (SML)\n\n"
        md += "| Property | Type | Default | Notes |\n|-|-|-|-|\n"
        md += "| id | identifier | - | Optional. Enables id-based event sugar (`on <id>.selected() { ... }`). |\n"
        md += "| text | string | \"\" | Display text. |\n"
        md += "| icon | string | \"\" | Optional icon resource/path. |\n"
        md += "| disabled | bool | false | Disables the option. |\n"
        md += "| selected | bool | false | If true, selects this option initially (first wins). |\n"

        md += "\n### Example\n\n"
        md += "```sml\n"
        md += "OptionButton { id: quality\n"
        md += "    Item { id: low; text: \"Low\" }\n"
        md += "    Item { id: med; text: \"Medium\"; selected: true }\n"
        md += "    Item { id: high; text: \"High\" }\n"
        md += "}\n"
        md += "```\n\n"

        md += "### SMS Event Examples\n\n"
        md += "```sms\n"
        md += "// With explicit item ids:\n"
        md += "on med.selected() { ... }\n\n"
        md += "// Container fallback (index based):\n"
        md += "on quality.itemSelected(index) { ... }\n"
        md += "```\n"

    # SML-only child structure for TabBar tabs (tabs are internal items, not child Controls).
    if c_name == "TabBar":
        md += "\n## SML Tabs\n\n"
        md += "`TabBar` tabs are defined as **SML child nodes** (pseudo elements).\n"
        md += "The runtime converts them to Godot TabBar tabs internally.\n\n"

        md += "### Supported tab elements\n\n"
        md += "- `Tab`\n\n"

        md += "### Tab properties (SML)\n\n"
        md += "| Property | Type | Default | Notes |\n|-|-|-|-|\n"
        md += "| id | identifier | - | Optional. Enables id-based event sugar (`on <id>.tabSelected() { ... }`). |\n"
        md += "| title | string | \"\" | Tab title. |\n"
        md += "| icon | string | \"\" | Optional icon resource/path. |\n"
        md += "| disabled | bool | false | Disables selecting the tab. |\n"
        md += "| hidden | bool | false | Hides the tab. |\n"
        md += "| selected | bool | false | If true, selects this tab initially (first wins). |\n"

        md += "\n### Example\n\n"
        md += "```sml\n"
        md += "TabBar { id: tabs\n"
        md += "    Tab { id: home; title: \"Home\"; selected: true }\n"
        md += "    Tab { id: settings; title: \"Settings\" }\n"
        md += "}\n"
        md += "```\n\n"

        md += "### SMS Event Examples\n\n"
        md += "```sms\n"
        md += "// With explicit tab ids:\n"
        md += "on home.tabSelected() { ... }\n\n"
        md += "// Container fallback (index based):\n"
        md += "on tabs.tabChanged(index) { ... }\n"
        md += "```\n"

    # Attached properties: only Controls can be children of DockingContainer / TabContainer.
    var is_control := c_name == "Control" or ClassDB.is_parent_class(c_name, "Control")
    var context_rules := _get_context_rules_for_child(c_name) if is_control else []
    if context_rules.size() > 0:
        md += "\n## Attached Properties\n\n"
        md += "These properties are declared by a parent provider and set on this element using the qualified syntax `<providerId>.property: value` or `ProviderType.property: value`.\n\n"

        for rule in context_rules:
            var parent_name := String(rule.get("parent", ""))
            var properties := Array(rule.get("properties", []))
            if parent_name == "" or properties.size() == 0:
                continue

            md += "### Provided by `%s`\n\n" % parent_name
            md += "| Attached Property | Type | Description |\n|-|-|-|\n"
            for p in properties:
                var p_name := String(p.get("sml", ""))
                if p_name == "":
                    continue
                var p_type := String(p.get("type", "Variant"))
                var p_desc := String(p.get("description", ""))
                md += "| %s | %s | %s |\n" % [p_name, p_type, p_desc]
            md += "\n"

    # Generic fallback: collection controls without a dedicated SML pseudo-child spec yet.
    if _is_collection_control(c_name) and c_name not in ["PopupMenu", "ItemList", "OptionButton", "TabBar", "TabContainer"]:
        md += "\n## SML Items (TODO)\n\n"
        md += "This control appears to manage internal items, but a dedicated SML pseudo-child specification has not been defined yet.\n"
        md += "Use generated runtime schema files (`ForgeRunner/Generated/Schema*.cs`) and this element reference as implementation hints.\n"

    var path := "%s/%s.md" % [OUT_DIR, c_name]
    var f := FileAccess.open(path, FileAccess.WRITE)
    if f:
        f.store_string(md)
        f.close()

func _inheritance_chain(c_name: String) -> Array[String]:
    # Manual/custom SML elements defined via specs
    if SPECS.has(c_name):
        var spec := Dictionary(SPECS[c_name])
        var backing := String(spec.get("backing", "Control"))
        var chain: Array[String] = [c_name]
        chain.append(backing)
        var cur: String = backing
        while cur != "":
            cur = String(ClassDB.get_parent_class(cur))
            if cur != "":
                chain.append(cur)
        return chain

    # Normal ClassDB chain
    var chain2: Array[String] = []
    var c: String = c_name
    while c != "":
        chain2.append(c)
        c = String(ClassDB.get_parent_class(c))
    return chain2


func _collect_properties(c_name: String) -> Array:
    if not ClassDB.class_exists(c_name):  # CHANGED
        return []  # CHANGED

    var parent := String(ClassDB.get_parent_class(c_name))

    var parent_names := {}
    if parent != "":
        for d in ClassDB.class_get_property_list(parent):
            var usage := int(d.get("usage", 0))  # CHANGED
            if (usage & PROPERTY_USAGE_EDITOR) == 0:  # CHANGED
                continue
            if (usage & PROPERTY_USAGE_CATEGORY) != 0: continue  # CHANGED
            if (usage & PROPERTY_USAGE_GROUP) != 0: continue  # CHANGED
            if (usage & PROPERTY_USAGE_SUBGROUP) != 0: continue  # CHANGED
            if (usage & PROPERTY_USAGE_INTERNAL) != 0: continue  # CHANGED
            if (usage & PROPERTY_USAGE_READ_ONLY) != 0: continue  # CHANGED

            var t := int(d.get("type", TYPE_NIL))  # CHANGED
            if not _is_sml_supported_type(t):  # CHANGED
                continue

            var n := String(d.get("name", ""))
            if n != "":
                parent_names[n] = true

    var collected := {}
    for d in ClassDB.class_get_property_list(c_name):
        var usage := int(d.get("usage", 0))  # CHANGED
        if (usage & PROPERTY_USAGE_EDITOR) == 0:  # CHANGED
            continue
        if (usage & PROPERTY_USAGE_CATEGORY) != 0: continue  # CHANGED
        if (usage & PROPERTY_USAGE_GROUP) != 0: continue  # CHANGED
        if (usage & PROPERTY_USAGE_SUBGROUP) != 0: continue  # CHANGED
        if (usage & PROPERTY_USAGE_INTERNAL) != 0: continue  # CHANGED
        if (usage & PROPERTY_USAGE_READ_ONLY) != 0: continue  # CHANGED

        var t := int(d.get("type", TYPE_NIL))  # CHANGED
        if not _is_sml_supported_type(t):  # CHANGED
            continue

        var name := String(d.get("name", ""))
        if name == "" or collected.has(name):
            continue
        if parent_names.has(name):
            continue

        collected[name] = {
            "name": name,
            "type": _type_name(t)
        }

    var out: Array = []
    for k in collected.keys():
        out.append(collected[k])
    out.sort_custom(func (a, b): return String(a["name"]) < String(b["name"]))
    return out

func _collect_signals(c_name: String) -> Array:
    if not ClassDB.class_exists(c_name):  # CHANGED
        return []  # CHANGED

    var parent := String(ClassDB.get_parent_class(c_name))

    var parent_names := {}
    if parent != "":
        for d in ClassDB.class_get_signal_list(parent):
            var n := String(d.get("name", ""))
            if n != "":
                parent_names[n] = true

    var collected := {}
    for d in ClassDB.class_get_signal_list(c_name):
        var name := String(d.get("name", ""))
        if name == "" or collected.has(name):
            continue
        if parent_names.has(name):
            continue

        var args: Array = d.get("args", [])
        var parts: Array[String] = []
        var names_only: Array[String] = []
        for a in args:
            var p_name := _normalize_param(String(a.get("name","arg")), name)
            parts.append("%s %s" % [_type_name(int(a.get("type", TYPE_NIL))), p_name])  # CHANGED
            names_only.append(p_name)  # CHANGED

        collected[name] = {
            "name": name,
            "params": (", ".join(parts) if parts.size() > 0 else "-"),
            "params_names": (", ".join(names_only) if names_only.size() > 0 else "")
        }

    var out: Array = []
    for k in collected.keys():
        out.append(collected[k])
    out.sort_custom(func (a, b): return String(a["name"]) < String(b["name"]))
    return out

#neu
func _collect_actions(c_name: String) -> Array:  # CHANGED
    if not ClassDB.class_exists(c_name):
        return []

    var parent := String(ClassDB.get_parent_class(c_name))

    var parent_names := {}
    if parent != "":
        for md in ClassDB.class_get_method_list(parent):
            var pn := String(md.get("name", ""))
            if pn != "":
                parent_names[pn] = true

    var all_prop_names := {}
    for pd in ClassDB.class_get_property_list(c_name):
        var prop_name := String(pd.get("name", ""))
        if prop_name != "":
            all_prop_names[prop_name] = true

    var collected := {}

    for m in ClassDB.class_get_method_list(c_name):
        var name := String(m.get("name", ""))
        if name == "" or name.begins_with("_"):
            continue
        if parent_names.has(name):
            continue

        if name.begins_with("set_") and all_prop_names.has(name.substr(4)):
            continue
        if name.begins_with("get_") and all_prop_names.has(name.substr(4)):
            continue
        if name.begins_with("is_") and all_prop_names.has(name.substr(3)):
            continue

        var returns := "void"
        var ret: Variant = m.get("return", null)  # CHANGED
        if typeof(ret) == TYPE_DICTIONARY:
            var rt: int = int(ret.get("type", TYPE_NIL))  # CHANGED
            if rt != TYPE_NIL:
                returns = _type_name(rt)

        var args: Array = Array(m.get("args", []))  # CHANGED
        var parts: Array[String] = []
        var names_only: Array[String] = []
        var supported := true

        for a in args:
            var t := int(a.get("type", TYPE_NIL))
            if not _is_sml_supported_type(t):
                supported = false
                break
            var p_name := _normalize_param(String(a.get("name", "arg")), name)
            parts.append("%s %s" % [_type_name(t), p_name])
            names_only.append(p_name)

        if not supported:
            continue

        collected[name] = {
            "name": name,
            "sms": _normalize_event(name),
            "params": (", ".join(parts) if parts.size() > 0 else "-"),
            "params_names": (", ".join(names_only) if names_only.size() > 0 else ""),
            "returns": returns
        }

    var out: Array = []
    for k in collected.keys():
        out.append(collected[k])

    out.sort_custom(func(a, b): return String(a["name"]) < String(b["name"]))
    return out


func _type_name(t: int) -> String:  # CHANGED
    match t:
        TYPE_BOOL: return "bool"
        TYPE_INT: return "int"
        TYPE_FLOAT: return "float"
        TYPE_STRING: return "string"
        TYPE_VECTOR2: return "Vector2"
        TYPE_VECTOR3: return "Vector3"
        TYPE_COLOR: return "Color"
        TYPE_OBJECT: return "Object"
        _: return "Variant"


# CHANGED: Only document properties that SML can represent and set.
func _is_sml_supported_type(t: int) -> bool:
    match t:
        TYPE_BOOL, TYPE_INT, TYPE_FLOAT, TYPE_STRING, TYPE_VECTOR2, TYPE_VECTOR3, TYPE_COLOR:
            return true
        _:
            return false


# Normalize a property name for SML (snake_case -> lowerCamelCase, and also lower first char).
func _normalize_property(name: String) -> String:
    return _to_lower_camel(name)

# Normalize a parameter name for SMS (snake_case -> lowerCamelCase).
# `signal_name` is the Godot signal that owns this parameter.
func _normalize_param(param_name: String, signal_name: String) -> String:
    return _to_lower_camel(param_name)  # CHANGED

# Default signal normalization (snake_case -> lowerCamelCase)
func _normalize_event(name: String) -> String:  # CHANGED
    return _to_lower_camel(name)



# Formats the full SMS handler syntax shown in docs.
# Example: Godot `button_up` -> `on <id>.buttonUp()`
func _format_sms_handler(godot_signal: String, params_names: String) -> String:
    var ev := _normalize_event(godot_signal)  # CHANGED
    if params_names == "":
        return "`on <id>." + ev + "() { ... }`"
    return "`on <id>." + ev + "(" + params_names + ") { ... }`"

# Shared lowerCamelCase conversion:
# - snake_case -> lowerCamelCase
# - PascalCase -> lowerCamelCase (e.g. Shortcut -> shortcut)
func _to_lower_camel(s: String) -> String:
    if s.find("_") != -1:
        var parts := s.split("_")
        if parts.size() == 0:
            return s
        var out := parts[0].to_lower()
        for i in range(1, parts.size()):
            out += parts[i].capitalize()
        return out

    # no underscores: just lowercase first char
    if s.length() == 0:
        return s
    return s.substr(0, 1).to_lower() + s.substr(1)


# Helper: collect subclasses of a class.
func _collect_subclasses(base: String, direct_only: bool) -> Array[String]:
    var out: Array[String] = []
    var classes := ClassDB.get_class_list()
    for c in classes:
        var cn := String(c)
        if cn == base:
            continue
        if direct_only:
            if String(ClassDB.get_parent_class(cn)) == base:
                out.append(cn)
        else:
            if ClassDB.is_parent_class(cn, base):
                out.append(cn)
    out.sort()
    return out

func _collect_manual_subclasses(base: String, direct_only: bool) -> Array[String]:
    var out: Array[String] = []
    # Manual types are defined by spec scripts in /tools/specs
    for mt in SPECS.keys():
        var spec := Dictionary(SPECS[mt])
        var backing := String(spec.get("backing", ""))
        if backing == "":
            continue

        if direct_only:
            if backing == base:
                out.append(String(mt))
        else:
            var chain := _inheritance_chain(String(mt))
            for i in range(1, chain.size()):
                if chain[i] == base:
                    out.append(String(mt))
                    break

    out.sort()
    return out

func _collect_subclasses_with_manual(base: String, direct_only: bool) -> Array[String]:
    var out := _collect_subclasses(base, direct_only)
    out.append_array(_collect_manual_subclasses(base, direct_only))
    out.sort()
    return out

func _md_link_list(names: Array[String]) -> String:
    if names.size() == 0:
        return "(none)\n"
    var lines: Array[String] = []
    for n in names:
        lines.append("- [" + n + "](" + n + ".md)")
    return "\n".join(lines) + "\n"
 
# CHANGED: In SML, only these controls are modeled as item-collections (pseudo children).
# Avoid heuristic detection (it flags too many normal controls, e.g. Button).
func _is_collection_control(c_name: String) -> bool:
    match c_name:
        "PopupMenu", "ItemList", "OptionButton", "TabBar", "TabContainer", "Tree", "MenuButton":
            return true
        _:
            return false

func _has_count_property(c_name: String) -> bool:
    for d in ClassDB.class_get_property_list(c_name):
        var n := String(d.get("name", ""))
        if n == "":
            continue
        if n == "item_count" or n == "tab_count":
            return true
        if n.ends_with("_count"):
            return true
    return false

func _has_item_methods(c_name: String) -> bool:
    for m in ClassDB.class_get_method_list(c_name):
        var mn := String(m.get("name", ""))
        if mn == "":
            continue
        if mn.begins_with("add_item"):
            return true
        if mn.begins_with("insert_item"):
            return true
        if mn.begins_with("remove_item"):
            return true
        if mn.begins_with("set_item_"):
            return true
        if mn.begins_with("get_item_"):
            return true
        if mn == "create_item":
            return true
        if mn.begins_with("add_tab") or mn.begins_with("set_tab_") or mn.begins_with("get_tab_"):
            return true
    return false

func _has_item_signals(c_name: String) -> bool:
    for s in ClassDB.class_get_signal_list(c_name):
        var sn := String(s.get("name", ""))
        if sn == "":
            continue
        # Common item/index/id signals
        if sn.find("item_") != -1:
            return true
        if sn.find("tab_") != -1:
            return true
        if sn == "id_pressed" or sn == "index_pressed":
            return true

        # Or any signal that exposes an index/id argument
        var args: Array = s.get("args", [])
        for a in args:
            var an := String(a.get("name", ""))
            if an == "index" or an == "id":
                return true
    return false


func _load_specs() -> Dictionary:
    var out := {}  # name -> spec dict

    var dir := DirAccess.open(SPEC_DIR)
    if dir == null:
        push_error("Spec dir not found: " + SPEC_DIR)
        return out

    dir.list_dir_begin()
    while true:
        var fn := dir.get_next()
        if fn == "":
            break
        if dir.current_is_dir():
            continue
        if not fn.ends_with(".gd"):
            continue

        var path := SPEC_DIR + "/" + fn
        var scr := load(path)
        if scr == null:
            push_error("Failed to load spec script: " + path)
            continue

        var inst = scr.new()
        if inst == null or not inst.has_method("get_spec"):
            push_error("Spec script missing get_spec(): " + path)
            continue

        var spec: Dictionary = inst.get_spec()
        var name := String(spec.get("name", ""))
        if name == "":
            push_error("Spec missing name: " + path)
            continue

        if name == "sms_functions":
            continue

        out[name] = spec

    dir.list_dir_end()
    return out

func _is_non_element_spec(spec_name: String) -> bool:
    return spec_name in ["sms_functions", "sml_resources", "context_properties", "layout_aliases", "layout_defaults", "runtime_overrides"]

func _get_context_rules() -> Array:
    if not SPECS.has("context_properties"):
        return []
    return Array(SPECS["context_properties"].get("rules", []))

func _get_context_rules_for_child(child_name: String) -> Array:
    var out: Array = []
    for rule in _get_context_rules():
        var child := String(rule.get("child", ""))
        if child != "*" and child.to_lower() != child_name.to_lower():
            continue
        out.append(rule)
    return out


func _get_spec_props(c_name: String) -> Array:  # CHANGED
    if not SPECS.has(c_name):
        return []
    return Array(SPECS[c_name].get("properties", []))


func _get_runtime_override_props(c_name: String) -> Array:
    if not SPECS.has("runtime_overrides"):
        return []

    var by_type := Dictionary(SPECS["runtime_overrides"].get("propertiesByType", {}))
    if not by_type.has(c_name):
        return []

    return Array(by_type.get(c_name, []))


func _get_runtime_override_actions(c_name: String) -> Array:
    if not SPECS.has("runtime_overrides"):
        return []

    var by_type := Dictionary(SPECS["runtime_overrides"].get("actionsByType", {}))
    if not by_type.has(c_name):
        return []

    return Array(by_type.get(c_name, []))


func _get_spec_actions(c_name: String) -> Array:  # CHANGED
    if not SPECS.has(c_name):
        return []
    return Array(SPECS[c_name].get("actions", []))


func _generate_csharp_schema_files(names: Array) -> void:
    _generate_schema_types_cs(names)
    _generate_schema_properties_cs(names)
    _generate_schema_events_cs(names)
    _generate_schema_context_properties_cs()
    _generate_schema_layout_aliases_cs()
    _generate_schema_layout_defaults_cs()

func _generate_schema_layout_defaults_cs() -> void:
    var cs := ""
    cs += "// <auto-generated />\n"
    cs += "#nullable enable\n"
    cs += "using System;\n"
    cs += "using System.Collections.Generic;\n\n"
    cs += "namespace Runtime.Generated;\n\n"
    cs += "public sealed record MenuBarDefaultsDef(int X, int Y, int Height, bool AnchorLeft, bool AnchorRight, bool AnchorTop, int MinHeight, int ZIndex);\n\n"
    cs += "public static class SchemaLayoutDefaults\n"
    cs += "{\n"
    var menu_bar_defaults := _get_menu_bar_defaults()
    var menu_x := int(menu_bar_defaults.get("x", 0))
    var menu_y := int(menu_bar_defaults.get("y", 0))
    var menu_height := int(menu_bar_defaults.get("height", 28))
    var menu_anchor_left := bool(menu_bar_defaults.get("anchorLeft", true))
    var menu_anchor_right := bool(menu_bar_defaults.get("anchorRight", true))
    var menu_anchor_top := bool(menu_bar_defaults.get("anchorTop", true))
    var menu_min_height := int(menu_bar_defaults.get("minHeight", 28))
    var menu_z_index := int(menu_bar_defaults.get("zIndex", 1000))

    cs += "    public static readonly MenuBarDefaultsDef MenuBar =\n"
    cs += "        new MenuBarDefaultsDef(%d, %d, %d, %s, %s, %s, %d, %d);\n" % [
        menu_x,
        menu_y,
        menu_height,
        ("true" if menu_anchor_left else "false"),
        ("true" if menu_anchor_right else "false"),
        ("true" if menu_anchor_top else "false"),
        menu_min_height,
        menu_z_index
    ]
    cs += "}\n"

    var path := GENERATED_DIR + "/SchemaLayoutDefaults.cs"
    var f := FileAccess.open(path, FileAccess.WRITE)
    if f:
        f.store_string(cs)
        f.close()

func _get_menu_bar_defaults() -> Dictionary:
    if not SPECS.has("layout_defaults"):
        return {}

    return Dictionary(SPECS["layout_defaults"].get("menuBarDefaults", {}))

func _get_layout_alias_rules() -> Array:
    if not SPECS.has("layout_aliases"):
        return []
    return Array(SPECS["layout_aliases"].get("rules", []))

func _generate_schema_layout_aliases_cs() -> void:
    var cs := ""
    cs += "// <auto-generated />\n"
    cs += "#nullable enable\n"
    cs += "using System;\n"
    cs += "using System.Collections.Generic;\n\n"
    cs += "namespace Runtime.Generated;\n\n"
    cs += "public sealed record LayoutAliasDef(string Canonical, string Alias, string Mode, string[] AppliesTo, string Precedence);\n\n"
    cs += "public static class SchemaLayoutAliases\n"
    cs += "{\n"
    cs += "    public static readonly LayoutAliasDef[] All =\n"
    cs += "    [\n"

    for rule in _get_layout_alias_rules():
        var canonical := String(rule.get("canonical", ""))
        var applies_to := Array(rule.get("appliesTo", []))
        var precedence := String(rule.get("precedence", "last-write-wins"))
        var aliases := Array(rule.get("aliases", []))
        if canonical == "" or aliases.size() == 0:
            continue

        var applies_to_values: Array[String] = []
        for t in applies_to:
            var type_name := String(t)
            if type_name != "":
                applies_to_values.append(type_name)

        for alias in aliases:
            var alias_name := ""
            var mode := "whole"

            if typeof(alias) == TYPE_DICTIONARY:
                alias_name = String(alias.get("name", ""))
                mode = String(alias.get("mode", "whole"))
            else:
                alias_name = String(alias)

            if alias_name == "":
                continue

            cs += "        new LayoutAliasDef(\"%s\", \"%s\", \"%s\", new string[] { %s }, \"%s\"),\n" % [
                _cs_string(canonical),
                _cs_string(alias_name),
                _cs_string(mode),
                _format_cs_string_array(applies_to_values),
                _cs_string(precedence)
            ]

    cs += "    ];\n\n"
    cs += "    private static readonly IReadOnlyDictionary<string, LayoutAliasDef> ByAlias = BuildByAlias();\n\n"
    cs += "    private static IReadOnlyDictionary<string, LayoutAliasDef> BuildByAlias()\n"
    cs += "    {\n"
    cs += "        var map = new Dictionary<string, LayoutAliasDef>(StringComparer.OrdinalIgnoreCase);\n"
    cs += "        foreach (var def in All)\n"
    cs += "        {\n"
    cs += "            map[def.Alias] = def;\n"
    cs += "        }\n"
    cs += "        return map;\n"
    cs += "    }\n\n"
    cs += "    public static bool TryGet(string alias, out LayoutAliasDef def)\n"
    cs += "        => ByAlias.TryGetValue(alias, out def!);\n"
    cs += "}\n"

    var path := GENERATED_DIR + "/SchemaLayoutAliases.cs"
    var f := FileAccess.open(path, FileAccess.WRITE)
    if f:
        f.store_string(cs)
        f.close()

func _generate_schema_context_properties_cs() -> void:
    var cs := ""
    cs += "// <auto-generated />\n"
    cs += "#nullable enable\n"
    cs += "using System;\n"
    cs += "using System.Collections.Generic;\n\n"
    cs += "namespace Runtime.Generated;\n\n"
    cs += "public sealed record ContextPropDef(string ParentType, string ChildType, string SmlName, string ValueType, string TargetMeta);\n\n"
    cs += "public static class SchemaContextProperties\n"
    cs += "{\n"
    cs += "    public static readonly ContextPropDef[] All =\n"
    cs += "    [\n"

    for rule in _get_context_rules():
        var parent_name := String(rule.get("parent", ""))
        var child_name := String(rule.get("child", ""))
        var properties := Array(rule.get("properties", []))
        if parent_name == "" or child_name == "":
            continue

        for p in properties:
            var sml_name := String(p.get("sml", ""))
            if sml_name == "":
                continue
            var value_type := String(p.get("type", "Variant"))
            var target_meta := String(p.get("targetMeta", sml_name))
            cs += "        new ContextPropDef(\"%s\", \"%s\", \"%s\", \"%s\", \"%s\"),\n" % [
                _cs_string(parent_name),
                _cs_string(child_name),
                _cs_string(sml_name),
                _cs_string(value_type),
                _cs_string(target_meta)
            ]

    cs += "    ];\n\n"
    cs += "    private static readonly IReadOnlyDictionary<string, ContextPropDef> ByKey = BuildByKey();\n\n"
    cs += "    private static IReadOnlyDictionary<string, ContextPropDef> BuildByKey()\n"
    cs += "    {\n"
    cs += "        var map = new Dictionary<string, ContextPropDef>(StringComparer.OrdinalIgnoreCase);\n"
    cs += "        foreach (var def in All)\n"
    cs += "        {\n"
    cs += "            map[BuildKey(def.ParentType, def.ChildType, def.SmlName)] = def;\n"
    cs += "        }\n"
    cs += "        return map;\n"
    cs += "    }\n\n"
    cs += "    public static bool TryGet(string parentType, string childType, string propertyName, out ContextPropDef def)\n"
    cs += "    {\n"
    cs += "        if (ByKey.TryGetValue(BuildKey(parentType, childType, propertyName), out def!))\n"
    cs += "        {\n"
    cs += "            return true;\n"
    cs += "        }\n"
    cs += "\n"
    cs += "        return ByKey.TryGetValue(BuildKey(parentType, \"*\", propertyName), out def!);\n"
    cs += "    }\n\n"
    cs += "    private static string BuildKey(string parentType, string childType, string propertyName)\n"
    cs += "        => parentType + \"|\" + childType + \"|\" + propertyName;\n"
    cs += "}\n"

    var path := GENERATED_DIR + "/SchemaContextProperties.cs"
    var f := FileAccess.open(path, FileAccess.WRITE)
    if f:
        f.store_string(cs)
        f.close()


func _generate_schema_types_cs(names: Array) -> void:
    var cs := ""
    cs += "// <auto-generated />\n"
    cs += "#nullable enable\n"
    cs += "using Runtime.Sml;\n"
    cs += "using System;\n"
    cs += "using System.Collections.Generic;\n\n"
    cs += "namespace Runtime.Generated;\n\n"
    cs += "public sealed record TypeDef(string Name, string Parent, bool IsCollection);\n\n"
    cs += "public static class SchemaTypes\n"
    cs += "{\n"
    cs += "    public static readonly TypeDef[] All =\n"
    cs += "    [\n"

    for n in names:
        var c_name := String(n)
        var parent := ""
        if SPECS.has(c_name):
            var backing := String(SPECS[c_name].get("backing", "Control"))
            parent = String(ClassDB.get_parent_class(backing)) if ClassDB.class_exists(backing) else backing
        elif ClassDB.class_exists(c_name):
            parent = String(ClassDB.get_parent_class(c_name))

        var is_collection := _is_collection_control(c_name)
        cs += "        new TypeDef(\"%s\", \"%s\", %s),\n" % [_cs_string(c_name), _cs_string(parent), ("true" if is_collection else "false")]

    cs += "    ];\n\n"
    cs += "    public static readonly IReadOnlyDictionary<string, TypeDef> TypesByName = BuildTypesByName();\n\n"
    cs += "    private static IReadOnlyDictionary<string, TypeDef> BuildTypesByName()\n"
    cs += "    {\n"
    cs += "        var map = new Dictionary<string, TypeDef>(StringComparer.OrdinalIgnoreCase);\n"
    cs += "        foreach (var def in All)\n"
    cs += "        {\n"
    cs += "            map[def.Name] = def;\n"
    cs += "        }\n"
    cs += "\n"
    cs += "        return map;\n"
    cs += "    }\n\n"
    cs += "    public static void RegisterKnownNodes(SmlParserSchema schema)\n"
    cs += "    {\n"
    cs += "        foreach (var def in All)\n"
    cs += "        {\n"
    cs += "            schema.RegisterKnownNode(def.Name);\n"
    cs += "        }\n"
    cs += "    }\n"
    cs += "}\n"

    var path := GENERATED_DIR + "/SchemaTypes.cs"
    var f := FileAccess.open(path, FileAccess.WRITE)
    if f:
        f.store_string(cs)
        f.close()


func _generate_schema_properties_cs(names: Array) -> void:
    var cs := ""
    cs += "// <auto-generated />\n"
    cs += "#nullable enable\n"
    cs += "using System;\n"
    cs += "using System.Collections.Generic;\n\n"
    cs += "namespace Runtime.Generated;\n\n"
    cs += "public sealed record PropDef(string TypeName, string SmlName, string GodotName, string ValueType);\n\n"
    cs += "public static class SchemaProperties\n"
    cs += "{\n"
    cs += "    public static readonly PropDef[] All =\n"
    cs += "    [\n"

    for n in names:
        var c_name := String(n)
        if SPECS.has(c_name):
            var spec_props := Array(SPECS[c_name].get("properties", []))
            for p in spec_props:
                var sml_name := String(p.get("sml", ""))
                if sml_name == "":
                    continue
                var t := String(p.get("type", "Variant"))
                var godot_name := String(p.get("godot", sml_name))
                cs += "        new PropDef(\"%s\", \"%s\", \"%s\", \"%s\"),\n" % [
                    _cs_string(c_name),
                    _cs_string(sml_name),
                    _cs_string(godot_name),
                    _cs_string(t)
                ]

        var runtime_override_props := _get_runtime_override_props(c_name)
        for p in runtime_override_props:
            var sml_name_override := String(p.get("sml", ""))
            if sml_name_override == "":
                continue
            var t_override := String(p.get("type", "Variant"))
            var godot_name_override := String(p.get("godot", sml_name_override))
            cs += "        new PropDef(\"%s\", \"%s\", \"%s\", \"%s\"),\n" % [
                _cs_string(c_name),
                _cs_string(sml_name_override),
                _cs_string(godot_name_override),
                _cs_string(t_override)
            ]

        if SPECS.has(c_name):
            continue

        if ClassDB.class_exists(c_name):
            for p in _collect_properties(c_name):
                var godot_name2 := String(p["name"])
                var sml_name2 := _normalize_property(godot_name2)
                var t2 := String(p["type"])
                cs += "        new PropDef(\"%s\", \"%s\", \"%s\", \"%s\"),\n" % [
                    _cs_string(c_name),
                    _cs_string(sml_name2),
                    _cs_string(godot_name2),
                    _cs_string(t2)
                ]

    cs += "    ];\n\n"
    cs += "    public static readonly IReadOnlyDictionary<string, IReadOnlyDictionary<string, PropDef>> PropsByType = BuildPropsByType();\n\n"
    cs += "    private static IReadOnlyDictionary<string, IReadOnlyDictionary<string, PropDef>> BuildPropsByType()\n"
    cs += "    {\n"
    cs += "        var map = new Dictionary<string, IReadOnlyDictionary<string, PropDef>>(StringComparer.OrdinalIgnoreCase);\n"
    cs += "        var buckets = new Dictionary<string, Dictionary<string, PropDef>>(StringComparer.OrdinalIgnoreCase);\n"
    cs += "\n"
    cs += "        foreach (var def in All)\n"
    cs += "        {\n"
    cs += "            if (!buckets.TryGetValue(def.TypeName, out var byProp))\n"
    cs += "            {\n"
    cs += "                byProp = new Dictionary<string, PropDef>(StringComparer.OrdinalIgnoreCase);\n"
    cs += "                buckets[def.TypeName] = byProp;\n"
    cs += "            }\n"
    cs += "\n"
    cs += "            byProp[def.SmlName] = def;\n"
    cs += "        }\n"
    cs += "\n"
    cs += "        foreach (var entry in buckets)\n"
    cs += "        {\n"
    cs += "            map[entry.Key] = entry.Value;\n"
    cs += "        }\n"
    cs += "\n"
    cs += "        return map;\n"
    cs += "    }\n"
    cs += "}\n"

    var path := GENERATED_DIR + "/SchemaProperties.cs"
    var f := FileAccess.open(path, FileAccess.WRITE)
    if f:
        f.store_string(cs)
        f.close()


func _generate_schema_events_cs(names: Array) -> void:
    var cs := ""
    cs += "// <auto-generated />\n"
    cs += "#nullable enable\n"
    cs += "using System;\n"
    cs += "using System.Collections.Generic;\n\n"
    cs += "namespace Runtime.Generated;\n\n"
    cs += "public sealed record EventDef(string TypeName, string SmsName, string GodotSignal, string[] ParamNames, string[] ParamTypes);\n\n"
    cs += "public static class SchemaEvents\n"
    cs += "{\n"
    cs += "    public static readonly EventDef[] All =\n"
    cs += "    [\n"

    for n in names:
        var c_name := String(n)
        if not ClassDB.class_exists(c_name):
            continue

        for s in _collect_signals(c_name):
            var godot_signal := String(s["name"])
            var sms_event := _normalize_event(godot_signal)
            var param_names := _split_param_names(String(s["params_names"]))
            var param_types := _split_param_types(String(s["params"]))

            cs += "        new EventDef(\"%s\", \"%s\", \"%s\", new string[] { %s }, new string[] { %s }),\n" % [
                _cs_string(c_name),
                _cs_string(sms_event),
                _cs_string(godot_signal),
                _format_cs_string_array(param_names),
                _format_cs_string_array(param_types)
            ]

    cs += "    ];\n\n"
    cs += "    public static readonly IReadOnlyDictionary<string, IReadOnlyDictionary<string, EventDef>> EventsByType = BuildEventsByType();\n\n"
    cs += "    private static IReadOnlyDictionary<string, IReadOnlyDictionary<string, EventDef>> BuildEventsByType()\n"
    cs += "    {\n"
    cs += "        var map = new Dictionary<string, IReadOnlyDictionary<string, EventDef>>(StringComparer.OrdinalIgnoreCase);\n"
    cs += "        var buckets = new Dictionary<string, Dictionary<string, EventDef>>(StringComparer.OrdinalIgnoreCase);\n"
    cs += "\n"
    cs += "        foreach (var def in All)\n"
    cs += "        {\n"
    cs += "            if (!buckets.TryGetValue(def.TypeName, out var byEvent))\n"
    cs += "            {\n"
    cs += "                byEvent = new Dictionary<string, EventDef>(StringComparer.OrdinalIgnoreCase);\n"
    cs += "                buckets[def.TypeName] = byEvent;\n"
    cs += "            }\n"
    cs += "\n"
    cs += "            byEvent[def.SmsName] = def;\n"
    cs += "        }\n"
    cs += "\n"
    cs += "        foreach (var entry in buckets)\n"
    cs += "        {\n"
    cs += "            map[entry.Key] = entry.Value;\n"
    cs += "        }\n"
    cs += "\n"
    cs += "        return map;\n"
    cs += "    }\n"
    cs += "}\n"

    var path := GENERATED_DIR + "/SchemaEvents.cs"
    var f := FileAccess.open(path, FileAccess.WRITE)
    if f:
        f.store_string(cs)
        f.close()


func _split_param_names(raw: String) -> Array[String]:
    var out: Array[String] = []
    var trimmed := raw.strip_edges()
    if trimmed == "" or trimmed == "-":
        return out

    var parts := trimmed.split(",", false)
    for p in parts:
        var t := String(p).strip_edges()
        if t != "":
            out.append(t)

    return out


func _split_param_types(raw: String) -> Array[String]:
    var out: Array[String] = []
    var trimmed := raw.strip_edges()
    if trimmed == "" or trimmed == "-":
        return out

    var parts := trimmed.split(",", false)
    for p in parts:
        var t := String(p).strip_edges()
        if t == "":
            continue
        var chunks := t.split(" ", false)
        if chunks.size() == 0:
            continue
        out.append(String(chunks[0]).strip_edges())

    return out


func _format_cs_string_array(values: Array[String]) -> String:
    if values.size() == 0:
        return ""

    var parts: Array[String] = []
    for v in values:
        parts.append("\"" + _cs_string(v) + "\"")
    return ", ".join(parts)


func _cs_string(raw: String) -> String:
    return raw.replace("\\", "\\\\").replace("\"", "\\\"")


# ---------------------------------------------------------------------------
# Native C++ header generation
# ---------------------------------------------------------------------------

func _generate_native_schema_files(names: Array) -> void:
    _emit_cpp_schema_types(names)
    _emit_cpp_schema_properties(names)
    _emit_cpp_schema_events(names)
    _emit_cpp_schema_context_properties()
    _emit_cpp_schema_layout_aliases()
    _emit_cpp_schema_layout_defaults()


func _cpp_string(raw: String) -> String:
    return raw.replace("\\", "\\\\").replace("\"", "\\\"")


func _emit_cpp_header(struct_decls: String, array_name: String, entries: String, struct_name: String) -> String:
    var h := "// <auto-generated - do not edit>\n"
    h += "#pragma once\n"
    h += "#include <string_view>\n\n"
    h += struct_decls + "\n"
    h += "inline constexpr %s %s[] = {\n" % [struct_name, array_name]
    h += entries
    h += "};\n"
    return h


func _emit_cpp_schema_types(names: Array) -> void:
    var struct_decl := "struct SchemaTypeDef { std::string_view name; std::string_view parent; bool is_collection; };"
    var entries := ""
    for n in names:
        var c_name := String(n)
        var parent := ""
        if SPECS.has(c_name):
            var backing := String(SPECS[c_name].get("backing", "Control"))
            parent = String(ClassDB.get_parent_class(backing)) if ClassDB.class_exists(backing) else backing
        elif ClassDB.class_exists(c_name):
            parent = String(ClassDB.get_parent_class(c_name))
        var is_collection := _is_collection_control(c_name)
        entries += "    {\"%s\", \"%s\", %s},\n" % [
            _cpp_string(c_name),
            _cpp_string(parent),
            "true" if is_collection else "false"
        ]
    var h := _emit_cpp_header(struct_decl, "kSchemaTypes", entries, "SchemaTypeDef")
    var f := FileAccess.open(NATIVE_GENERATED_DIR + "/schema_types.h", FileAccess.WRITE)
    if f:
        f.store_string(h)
        f.close()


func _emit_cpp_schema_properties(names: Array) -> void:
    var struct_decl := "struct SchemaPropDef { std::string_view type_name; std::string_view sml_name; std::string_view godot_name; std::string_view value_type; };"
    var entries := ""
    for n in names:
        var c_name := String(n)
        if SPECS.has(c_name):
            for p in Array(SPECS[c_name].get("properties", [])):
                var sml_name := String(p.get("sml", ""))
                if sml_name == "":
                    continue
                var godot_name := String(p.get("godot", sml_name))
                var t := String(p.get("type", "Variant"))
                entries += "    {\"%s\", \"%s\", \"%s\", \"%s\"},\n" % [
                    _cpp_string(c_name), _cpp_string(sml_name),
                    _cpp_string(godot_name), _cpp_string(t)
                ]

        for p in _get_runtime_override_props(c_name):
            var sml_name := String(p.get("sml", ""))
            if sml_name == "":
                continue
            var godot_name := String(p.get("godot", sml_name))
            var t := String(p.get("type", "Variant"))
            entries += "    {\"%s\", \"%s\", \"%s\", \"%s\"},\n" % [
                _cpp_string(c_name), _cpp_string(sml_name),
                _cpp_string(godot_name), _cpp_string(t)
            ]

        if SPECS.has(c_name):
            continue
        if ClassDB.class_exists(c_name):
            for p in _collect_properties(c_name):
                var godot_name := String(p["name"])
                var sml_name := _normalize_property(godot_name)
                var t := String(p["type"])
                entries += "    {\"%s\", \"%s\", \"%s\", \"%s\"},\n" % [
                    _cpp_string(c_name), _cpp_string(sml_name),
                    _cpp_string(godot_name), _cpp_string(t)
                ]

    var h := _emit_cpp_header(struct_decl, "kSchemaProperties", entries, "SchemaPropDef")
    var f := FileAccess.open(NATIVE_GENERATED_DIR + "/schema_properties.h", FileAccess.WRITE)
    if f:
        f.store_string(h)
        f.close()


func _emit_cpp_schema_events(names: Array) -> void:
    var struct_decl := "struct SchemaEventDef { std::string_view type_name; std::string_view sms_name; std::string_view godot_signal; };"
    var entries := ""
    for n in names:
        var c_name := String(n)
        if not ClassDB.class_exists(c_name):
            continue
        for s in _collect_signals(c_name):
            var godot_signal := String(s["name"])
            var sms_event := _normalize_event(godot_signal)
            entries += "    {\"%s\", \"%s\", \"%s\"},\n" % [
                _cpp_string(c_name), _cpp_string(sms_event), _cpp_string(godot_signal)
            ]
    var h := _emit_cpp_header(struct_decl, "kSchemaEvents", entries, "SchemaEventDef")
    var f := FileAccess.open(NATIVE_GENERATED_DIR + "/schema_events.h", FileAccess.WRITE)
    if f:
        f.store_string(h)
        f.close()


func _emit_cpp_schema_context_properties() -> void:
    var struct_decl := "struct SchemaContextPropDef { std::string_view parent_type; std::string_view child_type; std::string_view sml_name; std::string_view value_type; std::string_view target_meta; };"
    var entries := ""
    for rule in _get_context_rules():
        var parent_name := String(rule.get("parent", ""))
        var child_name := String(rule.get("child", ""))
        var properties := Array(rule.get("properties", []))
        if parent_name == "" or child_name == "":
            continue
        for p in properties:
            var sml_name := String(p.get("sml", ""))
            if sml_name == "":
                continue
            var value_type := String(p.get("type", "Variant"))
            var target_meta := String(p.get("targetMeta", sml_name))
            entries += "    {\"%s\", \"%s\", \"%s\", \"%s\", \"%s\"},\n" % [
                _cpp_string(parent_name), _cpp_string(child_name),
                _cpp_string(sml_name), _cpp_string(value_type), _cpp_string(target_meta)
            ]
    var h := _emit_cpp_header(struct_decl, "kSchemaContextProperties", entries, "SchemaContextPropDef")
    var f := FileAccess.open(NATIVE_GENERATED_DIR + "/schema_context_properties.h", FileAccess.WRITE)
    if f:
        f.store_string(h)
        f.close()


func _emit_cpp_schema_layout_aliases() -> void:
    var struct_decl := "struct SchemaLayoutAliasDef { std::string_view canonical; std::string_view alias; std::string_view mode; std::string_view precedence; };"
    var entries := ""
    for rule in _get_layout_alias_rules():
        var canonical := String(rule.get("canonical", ""))
        var aliases := Array(rule.get("aliases", []))
        var precedence := String(rule.get("precedence", "last-write-wins"))
        if canonical == "" or aliases.size() == 0:
            continue
        for alias in aliases:
            var alias_name := ""
            var mode := "whole"
            if typeof(alias) == TYPE_DICTIONARY:
                alias_name = String(alias.get("name", ""))
                mode = String(alias.get("mode", "whole"))
            else:
                alias_name = String(alias)
            if alias_name == "":
                continue
            entries += "    {\"%s\", \"%s\", \"%s\", \"%s\"},\n" % [
                _cpp_string(canonical), _cpp_string(alias_name),
                _cpp_string(mode), _cpp_string(precedence)
            ]
    var h := _emit_cpp_header(struct_decl, "kSchemaLayoutAliases", entries, "SchemaLayoutAliasDef")
    var f := FileAccess.open(NATIVE_GENERATED_DIR + "/schema_layout_aliases.h", FileAccess.WRITE)
    if f:
        f.store_string(h)
        f.close()


func _emit_cpp_schema_layout_defaults() -> void:
    var menu_bar_defaults := _get_menu_bar_defaults()
    var menu_x := int(menu_bar_defaults.get("x", 0))
    var menu_y := int(menu_bar_defaults.get("y", 0))
    var menu_height := int(menu_bar_defaults.get("height", 28))
    var menu_anchor_left := bool(menu_bar_defaults.get("anchorLeft", true))
    var menu_anchor_right := bool(menu_bar_defaults.get("anchorRight", true))
    var menu_anchor_top := bool(menu_bar_defaults.get("anchorTop", true))
    var menu_min_height := int(menu_bar_defaults.get("minHeight", 28))
    var menu_z_index := int(menu_bar_defaults.get("zIndex", 1000))

    var h := "// <auto-generated - do not edit>\n"
    h += "#pragma once\n\n"
    h += "struct SchemaMenuBarDefaultsDef {\n"
    h += "    int x, y, height, min_height, z_index;\n"
    h += "    bool anchor_left, anchor_right, anchor_top;\n"
    h += "};\n\n"
    h += "inline constexpr SchemaMenuBarDefaultsDef kMenuBarDefaults = {\n"
    h += "    %d, %d, %d, %d, %d,\n" % [menu_x, menu_y, menu_height, menu_min_height, menu_z_index]
    h += "    %s, %s, %s\n" % [
        "true" if menu_anchor_left else "false",
        "true" if menu_anchor_right else "false",
        "true" if menu_anchor_top else "false"
    ]
    h += "};\n"
    var f := FileAccess.open(NATIVE_GENERATED_DIR + "/schema_layout_defaults.h", FileAccess.WRITE)
    if f:
        f.store_string(h)
        f.close()