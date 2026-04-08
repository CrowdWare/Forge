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
        "name": "Viewport3D",
        "backing": "SubViewportContainer",
        "properties": [
            {"sml":"id", "type":"identifier", "default":"-"},
            {"sml":"model", "type":"string(url)", "default":"\"\""},
            {"sml":"modelSource", "type":"string(url)", "default":"\"\""},
            {"sml":"animation", "type":"string(url)", "default":"\"\""},
            {"sml":"animationSource", "type":"string(url)", "default":"\"\""},
            {"sml":"playAnimation", "type":"int", "default":"0"},
            {"sml":"playFirstAnimation", "type":"bool", "default":"false"},
            {"sml":"autoplayAnimation", "type":"bool", "default":"false"},
            {"sml":"defaultAnimation", "type":"string", "default":"\"\""},
            {"sml":"playLoop", "type":"bool", "default":"false"},
            {"sml":"cameraDistance", "type":"int", "default":"0"},
            {"sml":"lightEnergy", "type":"int", "default":"0"},
        ],
        "actions": [
            {"sms":"playAnimation", "params":[{"name":"index","type":"int"}], "returns":"void"},
            {"sms":"stopAnimation", "params":[], "returns":"void"},
            {"sms":"rewind", "params":[], "returns":"void"},
            {"sms":"setFrame", "params":[{"name":"frame","type":"int"}], "returns":"void"},
            {"sms":"zoomIn", "params":[], "returns":"void"},
            {"sms":"zoomOut", "params":[], "returns":"void"},
        ],
    }