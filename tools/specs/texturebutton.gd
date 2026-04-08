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

func get_spec() -> Dictionary:
    return {
        "name": "TextureButton",
        "backing": "TextureButton",
        "notes": [
            "A button that displays textures for each interaction state instead of a text label.",
            "All texture properties accept the same path formats as TextureRect: res:/, user:/, appRes:/, file://, or absolute paths.",
        ],
        "properties": [
            {"sml":"id",               "type":"identifier",   "default":"-"},
            {"sml":"textureNormal",    "type":"string(url)",  "default":"\"\"",
             "notes":"Texture shown in the default (idle) state."},
            {"sml":"textureHover",     "type":"string(url)",  "default":"\"\"",
             "notes":"Texture shown when the mouse hovers over the button. Alias: textureHovered."},
            {"sml":"texturePressed",   "type":"string(url)",  "default":"\"\"",
             "notes":"Texture shown while the button is held down."},
            {"sml":"textureDisabled",  "type":"string(url)",  "default":"\"\"",
             "notes":"Texture shown when the button is disabled."},
            {"sml":"textureFocused",   "type":"string(url)",  "default":"\"\"",
             "notes":"Texture shown when the button has keyboard focus."},
            {"sml":"disabled",            "type":"bool", "default":"false"},
            {"sml":"ignoreTextureSize",  "type":"bool", "default":"false",
             "notes":"If true, the texture size is not used for minimum size calculation. Required when using anchors or explicit width/height to constrain the button to a fixed size."},
        ],
        "examples_sml": [
            "TextureButton {",
            "    id: myBtn",
            "    textureNormal:  \"appRes:/assets/icons/bell.png\"",
            "    textureHover:   \"appRes:/assets/icons/bell_hover.png\"",
            "    texturePressed: \"appRes:/assets/icons/bell_pressed.png\"",
            "}",
        ],
    }
