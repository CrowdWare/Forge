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
        "name": "DockingContainer",
        "backing": "PanelContainer",
        "backing_native": "ForgeDockingContainerControl",  # C++ GDExtension class (TabContainer subclass)
        "properties": [
            {"sml":"id", "type":"identifier", "default":"-"},
            {"sml":"dockSide", "type":"enum", "default":"center"},
            {"sml":"fixedWidth", "type":"int", "default":"240"},
            {"sml":"minFixedWidth", "type":"int", "default":"140"},
            {"sml":"fixedHeight", "type":"int", "default":"-"},
            {"sml":"minHeight", "type":"int", "default":"-"},
            {"sml":"minFixedHeight", "type":"int", "default":"80"},
            {"sml":"heightPercent", "type":"float", "default":"50"},
            {"sml":"flex", "type":"bool", "default":"false"},
            {"sml":"collapsed", "type":"bool", "default":"false"},
            {"sml":"closeable", "type":"bool", "default":"true"},
            {"sml":"dragToRearrangeEnabled", "type":"bool", "default":"true"},
            {"sml":"tabsRearrangeGroup", "type":"int", "default":"1"}
        ],
        "actions": [],
        "notes": [
            "Automatically creates an internal TabContainer.",
            "Direct child controls become tabs. Use the attached property syntax to define tab captions: `<containerId>.title: \"Caption\"` or `DockingContainer.title: \"Caption\"`.",
            "Attached properties are resolved by instance id first, then by type name. The qualifier must refer to the direct parent DockingContainer.",
            "dockSide supports: farLeft, farLeftBottom, left, leftBottom, center, right, rightBottom, farRight, farRightBottom.",
            "For split columns (top/bottom), height can be controlled via fixedHeight (px) or heightPercent (0..100).",
            "Priority: fixedHeight > heightPercent > automatic 50/50 fallback.",
            "When flex is true, fixedWidth is ignored (warning emitted). Use minWidth to enforce a lower bound.",
            "collapsed:true hides the docking container from layout until shown or moved into.",
            "Use enum syntax without quotes, e.g. dockSide: left.",
            "dragToRearrangeEnabled: false excludes this container from docking move targets (kebab menu).",
            "A container is not listed as move target for itself (same dock slot is filtered)."
        ],
        "examples_sml": [
            "DockingContainer {",
            "    id: leftDock",
            "    dockSide: left",
            "    fixedWidth: 280",
            "    dragToRearrangeEnabled: true",
            "    tabsRearrangeGroup: 1",
            "",
            "    VBoxContainer {",
            "        id: project",
            "        leftDock.title: \"Project\"",
            "    }",
            "",
            "    VBoxContainer {",
            "        id: search",
            "        leftDock.title: \"Search\"",
            "    }",
            "}",
            "",
            "// Alternatively qualify by type name:",
            "DockingContainer {",
            "    id: rightDock",
            "    dockSide: right",
            "    fixedWidth: 360",
            "",
            "    VBoxContainer {",
            "        id: inspector",
            "        DockingContainer.title: \"Inspector\"",
            "    }",
            "}",
        ]
    }
