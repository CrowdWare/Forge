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
        "name": "DockingHost",
        "backing": "Container",
        "backing_native": "ForgeDockingHostControl",  # C++ GDExtension class (Container subclass)
        "properties": [
            {"sml":"id", "type":"identifier", "default":"-"},
            {"sml":"gap", "type":"int", "default":"0"},
            {"sml":"endGap", "type":"int", "default":"30"},
            {"sml":"anchors", "type":"string", "default":"\"\""},
            {"sml":"x", "type":"int", "default":"0"},
            {"sml":"y", "type":"int", "default":"0"},
            {"sml":"width", "type":"int", "default":"0"},
            {"sml":"height", "type":"int", "default":"0"}
        ],
        "actions": [],
        "notes": [
            "Layouts multiple DockingContainer children using dockSide/fixedWidth/flex semantics.",
            "Supports side columns with optional bottom companions (farLeftBottom, leftBottom, rightBottom, farRightBottom).",
            "If only top or bottom panel of a side is present/visible, that panel fills the full host height.",
            "If both are present/visible, top and bottom split the host height 50/50.",
            "Resize handles are created per interior gap as upper and lower segments to keep middle gap area clickable for menu buttons.",
            "Typically used as a full-rect region inside a Window."
        ],
        "examples_sml": [
            "DockingHost {",
            "    id: mainDockHost",
            "    anchors: left | top | right | bottom",
            "    gap: 8",
            "    endGap: 30",
            "}"
        ]
    }
