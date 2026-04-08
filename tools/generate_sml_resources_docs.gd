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

var REPO_ROOT := ""
var SPEC_DIR := ""
var OUT_PATH := ""

func _initialize() -> void:
    REPO_ROOT = ProjectSettings.globalize_path("res://") + "/../.."
    SPEC_DIR = REPO_ROOT + "/tools/specs"
    OUT_PATH = REPO_ROOT + "/docs/sml_resources.md"
    generate()
    quit()

func generate() -> void:
    var spec_path := SPEC_DIR + "/resources.gd"
    var spec_res := load(spec_path)
    if spec_res == null:
        push_error("Failed to load resources spec: " + spec_path)
        return

    var syntax: Array = spec_res.SYNTAX
    var namespaces: Dictionary = spec_res.NAMESPACES
    var localization: Dictionary = spec_res.LOCALIZATION

    var md := "# SML Resource System\n\n"
    md += "Resource references let SML properties pull values from named namespaces - "
    md += "most commonly translated strings from locale-aware files.\n\n"

    # --- Syntax ---
    md += "## Syntax\n\n"
    for entry in syntax:
        var name := String(entry.get("name", ""))
        var syn := String(entry.get("syntax", ""))
        var desc := String(entry.get("description", ""))
        var example := String(entry.get("example", ""))
        md += "### %s\n\n" % name
        md += "```\n%s\n```\n\n" % syn
        md += "%s\n\n" % desc
        if example != "":
            md += "**Example:**\n```sml\n%s\n```\n\n" % example

    # --- Namespaces ---
    md += "## Namespaces\n\n"
    md += "Four namespaces are built into SML. Well-known namespaces do not generate "
    md += "validation warnings when no inline block is present, because they can be resolved from external files at runtime.\n\n"

    var ns_names := namespaces.keys()
    ns_names.sort()
    for ns_name in ns_names:
        var ns: Dictionary = namespaces[ns_name]
        var desc := String(ns.get("description", ""))
        var resolution: Array = ns.get("resolution", [])
        var files: Array = ns.get("files", [])
        var example := String(ns.get("example", ""))

        md += "### %s\n\n" % ns_name
        md += "%s\n\n" % desc

        if resolution.size() > 0:
            md += "**Resolution order:**\n\n"
            for step in resolution:
                md += "- %s\n" % String(step)
            md += "\n"

        if files.size() > 0:
            md += "**Files:**\n\n"
            md += "| File | Role |\n"
            md += "|---|---|\n"
            for f in files:
                var fname := String(f.get("name", ""))
                var role := String(f.get("role", ""))
                md += "| `%s` | %s |\n" % [fname, role]
            md += "\n"

        if example != "":
            md += "**Example block:**\n```sml\n%s\n```\n\n" % example

    # --- Localization ---
    md += "## Localization (Strings namespace)\n\n"
    md += "%s\n\n" % String(localization.get("description", ""))

    md += "**File format:** %s\n\n" % String(localization.get("file_format", ""))
    md += "**File placement:** %s\n\n" % String(localization.get("file_placement", ""))
    md += "**Locale selection:** %s\n\n" % String(localization.get("locale_selection", ""))

    var loc_example := String(localization.get("example_file", ""))
    if loc_example != "":
        md += "**Example (strings-de.sml):**\n```sml\n%s\n```\n\n" % loc_example

    # --- Validation ---
    md += "## Validation\n\n"
    md += "The SML parser emits warnings at parse time for unresolved references:\n\n"
    md += "| Case | Warning emitted? |\n"
    md += "|---|---|\n"
    md += "| `@Strings.key` - no inline block, no fallback | No (Strings is a well-known external namespace) |\n"
    md += "| `@Strings.key` - inline block present, key missing, no fallback | Yes |\n"
    md += "| `@UnknownNS.key` - unknown namespace, no fallback | Yes |\n"
    md += "| Any reference with a fallback literal | No |\n"
    md += "\n"

    var file := FileAccess.open(OUT_PATH, FileAccess.WRITE)
    if file:
        file.store_string(md)
        file.close()
        print("SML resource system documentation generated: " + OUT_PATH)
    else:
        push_error("Failed to write SML resources documentation to: " + OUT_PATH)


static func get_spec() -> Dictionary:
    return {
        "name": "sml_resources",
        "elements": []
    }
