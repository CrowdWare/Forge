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
        "name": "WindowDrag",
        "backing": "Panel",
        "properties": [
            {"sml":"id", "type":"identifier", "default":"-"},
            {"sml":"anchors", "type":"string", "default":"\"\""},
            {"sml":"x", "type":"int", "default":"0"},
            {"sml":"y", "type":"int", "default":"0"},
            {"sml":"width", "type":"int", "default":"0"},
            {"sml":"height", "type":"int", "default":"32"}
        ],
        "actions": [],
        "notes": [
            "Provides a native draggable title/caption area for custom frameless layouts.",
            "Single left click starts OS window drag.",
            "Double left click toggles maximize/restore (windowed <-> maximized).",
            "Useful together with Window.extendToTitle: true.",
            "When used with DockingHost, set WindowDrag.height analogous to DockingHost.offsetTop so caption drag area and dock top offset align."
        ],
        "examples_sml": [
            "Window {",
            "    id: mainWindow",
            "    extendToTitle: true",
            "",
            "    WindowDrag {",
            "        id: titleDrag",
            "        anchors: left | top | right",
            "        height: 34",
            "    }",
            "}"
        ]
    }
