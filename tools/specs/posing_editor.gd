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
        "name": "PosingEditor",
        "backing": "SubViewportContainer",
        "properties": [
            {"sml":"id",              "type":"identifier", "default":"-"},
            {"sml":"src",             "type":"string(url)", "default":"\"\""},
            {"sml":"showBoneTree",    "type":"bool",        "default":"false"},
            {"sml":"normalizeNames",  "type":"bool",        "default":"true"},
        ],
        "children": [
            {
                "name": "JointConstraint",
                "properties": [
                    {"sml":"bone", "type":"string",  "default":"\"\""},
                    {"sml":"minX", "type":"float",   "default":"-180"},
                    {"sml":"maxX", "type":"float",   "default":"180"},
                    {"sml":"minY", "type":"float",   "default":"-180"},
                    {"sml":"maxY", "type":"float",   "default":"180"},
                    {"sml":"minZ", "type":"float",   "default":"-180"},
                    {"sml":"maxZ", "type":"float",   "default":"180"},
                ],
            },
        ],
        "actions": [
            {"sms":"resetPose",           "params":[], "returns":"void"},
            {"sms":"setJointSpheresVisible", "params":[{"name":"visible","type":"bool"}], "returns":"void"},
            {"sms":"setBoneTree",         "params":[{"name":"treeId","type":"string"}], "returns":"void"},
            {"sms":"setModelSource",      "params":[{"name":"src","type":"string"}], "returns":"void"},
            {"sms":"loadProject",         "params":[{"name":"path","type":"string"}], "returns":"void"},
            {"sms":"getProjectText",      "params":[{"name":"path","type":"string"}], "returns":"string", "note":"Reads raw .scene source text from disk (absolute paths allowed)."},
            {"sms":"applyProjectText",    "params":[{"name":"path","type":"string"},{"name":"content","type":"string"}], "returns":"bool", "note":"Parses scene source text, applies it to viewport/timeline, and writes to disk only on successful parse."},
            {"sms":"saveProject",         "params":[{"name":"path","type":"string"}], "returns":"void"},
            {"sms":"setProjectProperty",  "params":[{"name":"key","type":"string"},{"name":"value","type":"string"}], "returns":"void", "note":"Stores scene-level metadata persisted in .scene root properties."},
            {"sms":"getProjectProperty",  "params":[{"name":"key","type":"string"},{"name":"fallback","type":"string"}], "returns":"string", "note":"Reads scene-level metadata from .scene root properties."},
            {"sms":"addSceneAsset",       "params":[{"name":"path","type":"string"},{"name":"posX","type":"float"},{"name":"posY","type":"float"},{"name":"posZ","type":"float"}], "returns":"int", "note":"Auto-detects rigged assets: returns 1 for Character, 0 for Prop, -1 on failure."},
            {"sms":"addGreyboxItem",     "params":[{"name":"kind","type":"string"},{"name":"posX","type":"float"},{"name":"posY","type":"float"},{"name":"posZ","type":"float"}], "returns":"int", "note":"Adds a built-in greyboxing prop (wall/tree/door/window). Returns prop index or -1 on failure."},
            {"sms":"addSceneProp",        "params":[{"name":"path","type":"string"},{"name":"posX","type":"float"},{"name":"posY","type":"float"},{"name":"posZ","type":"float"}], "returns":"int"},
            {"sms":"getSceneCharacterCount","params":[], "returns":"int"},
            {"sms":"getActiveCharacterId","params":[], "returns":"string", "note":"Returns currently active character id used for posing/keyframe routing."},
            {"sms":"getSceneCharacterPath","params":[{"name":"index","type":"int"}], "returns":"string"},
            {"sms":"getSceneCharacterName","params":[{"name":"index","type":"int"}], "returns":"string"},
            {"sms":"getSceneCharacterVisible","params":[{"name":"index","type":"int"}], "returns":"bool"},
            {"sms":"selectSceneCharacter","params":[{"name":"index","type":"int"}], "returns":"void"},
            {"sms":"removeCharacter",     "params":[], "returns":"void"},
            {"sms":"removeSceneCharacter","params":[{"name":"index","type":"int"}], "returns":"void"},
            {"sms":"removeSceneProp",     "params":[{"name":"index","type":"int"}], "returns":"void"},
            {"sms":"getScenePropCount",   "params":[], "returns":"int"},
            {"sms":"getScenePropPath",    "params":[{"name":"index","type":"int"}], "returns":"string"},
            {"sms":"getScenePropName",    "params":[{"name":"index","type":"int"}], "returns":"string"},
            {"sms":"getScenePropVisible","params":[{"name":"index","type":"int"}], "returns":"bool"},
            {"sms":"getScenePropPos",     "params":[{"name":"index","type":"int"}], "returns":"string", "note":"Returns 'x,y,z' string"},
            {"sms":"setScenePropPos",     "params":[{"name":"index","type":"int"},{"name":"x","type":"float"},{"name":"y","type":"float"},{"name":"z","type":"float"}], "returns":"void"},
            {"sms":"setSceneCharacterVisible","params":[{"name":"index","type":"int"},{"name":"visible","type":"bool"}], "returns":"void"},
            {"sms":"setScenePropVisible","params":[{"name":"index","type":"int"},{"name":"visible","type":"bool"}], "returns":"void"},
            {"sms":"getScenePropRot",     "params":[{"name":"index","type":"int"}], "returns":"string", "note":"Returns 'x,y,z' Euler degrees"},
            {"sms":"setScenePropRot",     "params":[{"name":"index","type":"int"},{"name":"x","type":"float"},{"name":"y","type":"float"},{"name":"z","type":"float"}], "returns":"void"},
            {"sms":"placeSelectedOnGround","params":[{"name":"groundY","type":"float"}], "returns":"bool", "note":"Moves selected character/prop so its world-space AABB bottom sits on groundY (use 0 for default floor)."},
            {"sms":"rebaseSelectedPivotBottom","params":[], "returns":"bool", "note":"Wraps selected character/prop in a pivot helper placed at world-space bottom-center; keeps visual transform unchanged."},
            {"sms":"getPoseDataForCharacter", "params":[{"name":"characterId","type":"string"}], "returns":"object", "note":"Returns scoped poseData dictionary for one character only."},
            {"sms":"getPoseDataForActiveCharacter", "params":[], "returns":"object", "note":"Returns scoped poseData dictionary for the currently active character."},
            {"sms":"getPoseDataForBone", "params":[{"name":"boneName","type":"string"}], "returns":"object", "note":"Returns scoped poseData dictionary for one specific bone of the active character."},
            {"sms":"getPoseDataForSelectedBone", "params":[], "returns":"object", "note":"Returns scoped poseData dictionary with only the currently selected bone of the active character."},
            {"sms":"setMode",             "params":[{"name":"mode","type":"string"}], "returns":"void", "note":"Values: 'pose' | 'arrange'. Clears all gizmos and selections."},
            {"sms":"setEditMode",         "params":[{"name":"mode","type":"string"}], "returns":"void", "note":"Values: 'move' | 'scale' | 'rotate'. Only effective in Arrange mode."},
            {"sms":"exportGlb",           "params":[{"name":"path","type":"string"},{"name":"includeAnimation","type":"bool"},{"name":"includeProps","type":"bool"},{"name":"animationOnlyCharacter","type":"bool"}], "returns":"bool", "note":"Starts async GLB export. Progress is reported via exportProgress."},
            {"sms":"exportCurrentFramePng","params":[{"name":"path","type":"string"}], "returns":"bool", "note":"Captures current viewport frame and writes a PNG."},
            {"sms":"exportFrameRangePng", "params":[{"name":"frameFrom","type":"int"},{"name":"frameTo","type":"int"},{"name":"outputDirectory","type":"string"}], "returns":"int", "note":"Exports a PNG sequence frame-by-frame from the timeline and returns written frame count."},
            {"sms":"startExportFrameRangePng", "params":[{"name":"frameFrom","type":"int"},{"name":"frameTo","type":"int"},{"name":"outputDirectory","type":"string"}], "returns":"bool", "note":"Starts async frame-by-frame PNG sequence export. Completion is signaled via frameRangeExportFinished."},
        ],
        "events": [
            {"sms":"boneSelected",    "params":[{"name":"boneName","type":"string"}]},
            {"sms":"poseChanged",     "params":[{"name":"boneName","type":"string"}]},
            {"sms":"poseReset",       "params":[]},
            {"sms":"scenePropAdded",  "params":[{"name":"index","type":"int"},{"name":"path","type":"string"}]},
            {"sms":"scenePropRemoved","params":[{"name":"index","type":"int"}]},
            {"sms":"objectSelected",  "params":[{"name":"propIdx","type":"int"}], "note":"-1 = deselect"},
            {"sms":"objectMoved",     "params":[{"name":"propIdx","type":"int"},{"name":"pos","type":"string"}], "note":"pos = 'x,y,z'"},
            {"sms":"frameRangeExportFinished", "params":[{"name":"written","type":"int"},{"name":"outputDirectory","type":"string"}]},
        ],
        "dynamic_properties": [
            {"sms":"poseData", "type":"object", "note":"Dictionary<boneName, Quaternion> - use with setKeyframe/loadPose"},
            {"sms":"currentFrame", "type":"int"},
        ],
    }
