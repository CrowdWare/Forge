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
        "name": "Timeline",
        "backing": "Control",
        "properties": [
            {"sml":"id",           "type":"identifier", "default":"-"},
            {"sml":"fps",          "type":"int",        "default":"24"},
            {"sml":"totalFrames",  "type":"int",        "default":"120"},
        ],
        "actions": [
            {"sms":"play",              "params":[], "returns":"void"},
            {"sms":"stop",              "params":[], "returns":"void"},
            {"sms":"setKeyframe",       "params":[{"name":"frame","type":"int"},{"name":"poseData","type":"object"}], "returns":"void"},
            {"sms":"setKeyframeBone",   "params":[{"name":"frame","type":"int"},{"name":"poseData","type":"object"},{"name":"boneName","type":"string"}], "returns":"void", "note":"Filters/sets only the specified bone key for the frame (used by poseChanged autokey)."},
            {"sms":"setKeyframeMove",   "params":[{"name":"frame","type":"int"}], "returns":"void", "note":"Stores active character move key (xyz) for pose-mode move autokey."},
            {"sms":"setKeyframeRotate", "params":[{"name":"frame","type":"int"}], "returns":"void", "note":"Stores active character pivot rotation key (xyz degrees) for pose-mode rotate autokey when no bone is selected."},
            {"sms":"removeKeyframe",    "params":[{"name":"frame","type":"int"}], "returns":"void"},
            {"sms":"getPoseAt",         "params":[{"name":"frame","type":"int"}], "returns":"object"},
            {"sms":"getKeyframeCount",  "params":[], "returns":"int"},
            {"sms":"getKeyframeFrameAt","params":[{"name":"index","type":"int"}], "returns":"int"},
            {"sms":"getKeyframeBoneCount","params":[{"name":"frame","type":"int"}], "returns":"int"},
            {"sms":"getKeyframeBoneName","params":[{"name":"frame","type":"int"},{"name":"boneIndex","type":"int"}], "returns":"string"},
            {"sms":"getKeyframeCountForCharacter","params":[{"name":"characterId","type":"string"}], "returns":"int"},
            {"sms":"getKeyframeFrameAtForCharacter","params":[{"name":"index","type":"int"},{"name":"characterId","type":"string"}], "returns":"int"},
            {"sms":"getKeyframeBoneCountForCharacter","params":[{"name":"frame","type":"int"},{"name":"characterId","type":"string"}], "returns":"int"},
            {"sms":"getKeyframeBoneNameForCharacter","params":[{"name":"frame","type":"int"},{"name":"boneIndex","type":"int"},{"name":"characterId","type":"string"}], "returns":"string"},
            {"sms":"setVisibleCharacterId","params":[{"name":"characterId","type":"string"}], "returns":"void", "note":"Filters timeline track display to one character id."},
            {"sms":"debugLogKeyframesForCharacter","params":[{"name":"characterId","type":"string"}], "returns":"void"},
            {"sms":"clearAllKeyframes", "params":[], "returns":"void"},
        ],
        "events": [
            {"sms":"frameChanged",     "params":[{"name":"frame","type":"int"}]},
            {"sms":"keyframeAdded",    "params":[{"name":"frame","type":"int"},{"name":"boneName","type":"string"}]},
            {"sms":"keyframeRemoved",  "params":[{"name":"frame","type":"int"}]},
            {"sms":"playbackStarted",  "params":[]},
            {"sms":"playbackStopped",  "params":[]},
        ],
        "dynamic_properties": [
            {"sms":"currentFrame", "type":"int"},
            {"sms":"fps",          "type":"int"},
            {"sms":"totalFrames",  "type":"int"},
        ],
    }
