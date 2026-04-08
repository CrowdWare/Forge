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

extends RefCounted

# SML Resource System Specification
# Defines the @Namespace.key reference syntax, supported namespaces, and localization rules.

const SYNTAX := [
    {
        "name": "Basic reference",
        "syntax": "@Namespace.key",
        "description": "Resolves the value of 'key' from the given namespace at runtime. For well-known namespaces (Strings, Colors, Icons, Layouts), no fallback is required - they are resolved from external files. A validation warning is emitted for unknown namespaces without a fallback.",
        "example": "title: @Strings.windowTitle"
    },
    {
        "name": "Reference with fallback",
        "syntax": "@Namespace.key, \"Fallback\"",
        "description": "Resolves the value if available; uses the literal fallback when the key cannot be resolved. Suppresses validation warnings for unknown namespaces or keys. The fallback must be a string, boolean, integer, or float literal.",
        "example": "text: @Strings.greeting, \"Hello\""
    }
]

const NAMESPACES := {
    "Strings": {
        "description": "Text strings for UI labels, titles, menu entries, and any other displayed text. The Strings namespace is the only one with built-in locale-aware file selection.",
        "resolution": [
            "1. Language-specific file (e.g. strings-de.sml) - overrides for the current system locale.",
            "2. Default file (strings.sml) - English or app-default strings.",
            "3. Fallback literal in the SML source (if provided).",
            "4. Empty string (with a runtime warning)."
        ],
        "files": [
            { "name": "strings.sml", "role": "Default/English strings. Always loaded." },
            { "name": "strings-{lang}.sml", "role": "Locale override, e.g. strings-de.sml for German. Loaded when the system locale matches." }
        ],
        "example": "Strings {\n    windowTitle: \"ForgeRunner\"\n    menuAppAbout: \"About ForgeRunner\"\n}"
    },
    "Colors": {
        "description": "Named color values for theming and styling. Design tokens are defined centrally in theme.sml files and are automatically available in all SML documents without an inline block.",
        "resolution": [
            "1. Inline Colors block in the same SML document (highest priority).",
            "2. App-level theme.sml in the same directory as app.sml (optional override).",
            "3. ForgeRunner default theme (res://theme.sml).",
            "4. Fallback literal (if provided).",
            "5. Empty string (with a runtime warning)."
        ],
        "files": [
            { "name": "theme.sml", "role": "App-level override. Optional. Place next to app.sml to customize tokens." },
            { "name": "res://theme.sml", "role": "ForgeRunner built-in default theme. Always loaded as fallback." }
        ],
        "example": "Colors {\n    primary: \"#3A7BD5\"\n    background: \"#1E1E2E\"\n}"
    },
    "Icons": {
        "description": "Icon resource paths for images and buttons. Resolved from an inline Icons block in the SML document.",
        "resolution": [
            "1. Inline Icons block in the same SML document.",
            "2. Fallback literal (if provided).",
            "3. Empty string (with a runtime warning)."
        ],
        "files": [],
        "example": "Icons {\n    save: \"res://assets/icons/save.svg\"\n    close: \"res://assets/icons/close.svg\"\n}"
    },
    "Layouts": {
        "description": "Layout configuration values such as sizes, paddings, and spacing constants. Design tokens are defined centrally in theme.sml files and are automatically available in all SML documents.",
        "resolution": [
            "1. Inline Layouts block in the same SML document (highest priority).",
            "2. App-level theme.sml in the same directory as app.sml (optional override).",
            "3. ForgeRunner default theme (res://theme.sml).",
            "4. Fallback literal (if provided).",
            "5. Empty string (with a runtime warning)."
        ],
        "files": [
            { "name": "theme.sml", "role": "App-level override. Optional. Place next to app.sml to customize tokens." },
            { "name": "res://theme.sml", "role": "ForgeRunner built-in default theme. Always loaded as fallback." }
        ],
        "example": "Layouts {\n    buttonPaddingH: 12\n    cornerRadius: 6\n}"
    },
    "Fonts": {
        "description": "Named font paths for use with fontFace and fontWeight properties. Keys follow the convention 'FamilyName-WeightAlias' (e.g. Roboto-Bold). The runtime looks up the key based on the fontFace + fontWeight combination and loads the font via AddThemeFontOverride.",
        "resolution": [
            "1. Inline Fonts block in the same SML document.",
            "2. Fallback: warning is logged and no font override is applied."
        ],
        "files": [],
        "example": "Fonts {\n    Roboto-Regular: \"res://fonts/Roboto-Regular.ttf\"\n    Roboto-Bold:    \"res://fonts/Roboto-Bold.ttf\"\n    Roboto-Light:   \"res://fonts/Roboto-Light.ttf\"\n}\n\nLabel {\n    fontFace: \"Roboto\"\n    fontWeight: bold\n    text: \"Hello\"\n}"
    }
}

const LOCALIZATION := {
    "description": "The Strings namespace resolves strings from locale-specific files placed alongside the SML document. Files are auto-discovered based on the system locale at startup.",
    "file_format": "A plain SML document containing a single Strings { } block with key-value pairs. Keys are camelCase identifiers; values are quoted strings.",
    "file_placement": "Strings files must be in the same directory as the app.sml that references them.",
    "locale_selection": "The runtime reads the two-letter ISO 639-1 language code from the system locale (e.g. 'de' from 'de_DE'). It then attempts to load strings-{lang}.sml. If the file does not exist or the locale is 'en', only strings.sml is used.",
    "example_file": "Strings {\n    windowTitle: \"ForgeRunner\"\n    menuAppAbout: \"Über ForgeRunner\"\n    menuAppQuit: \"ForgeRunner beenden\"\n}"
}

# This file is not an SML element spec - return an empty spec so the element doc
# generator can load it without errors.
static func get_spec() -> Dictionary:
    return {
        "name": "sml_resources",
        "elements": []
    }
