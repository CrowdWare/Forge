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
        "name": "SplashScreen",
        "backing": "Panel",
        "notes": [
            "Startup screen shown before the main app loads. Shown immediately after entry files are downloaded. Remaining assets load in background with an optional ProgressBar child.",
            "SplashScreen is always centered on screen and uses extendToTitle so the content fills the title bar area while the system close button remains visible. These behaviours are implicit - no properties needed.",
        ],
        "properties": [
            {"sml":"id",            "type":"identifier",   "default":"-"},
            {"sml":"title",         "type":"string",       "default":"\"\""},
            {"sml":"size",          "type":"vec2i",        "default":"640, 480"},
            {"sml":"pos",           "type":"vec2i",        "default":"0, 0"},
            {"sml":"minSize",       "type":"vec2i",        "default":"0, 0"},
            {"sml":"duration",      "type":"int",          "default":"0",
             "notes":"Minimum display time in milliseconds before loading the next document"},
            {"sml":"loadOnReady",   "type":"string(url)",  "default":"\"\"",
             "notes":"SML document URL to load after duration has elapsed and all assets are ready"},
        ],
        "examples_sml": [
            "SplashScreen {",
            "    id: splash",
            "    size: 640, 480",
            "    duration: 3000",
            "    loadOnReady: \"res://docs/Default/main.sml\"",
            "",
            "    Label { text: \"Loading...\" }",
            "    ProgressBar {",
            "        id: downloadProgress",
            "        showPercentage: false",
            "        visible: false",
            "    }",
            "}",
        ],
    }
