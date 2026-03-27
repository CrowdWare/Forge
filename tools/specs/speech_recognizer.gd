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
        "name": "SpeechRecognizer",
        "backing": "Control",
        "backing_native": "ForgeSpeechRecognizerControl",
        "properties": [
            {"sml":"id", "type":"identifier", "default":"—"},
            {"sml":"language", "type":"string", "default":"de-DE"},
            {"sml":"mode", "type":"enum: raw, clean, markdown", "default":"clean"},
            {"sml":"filters", "type":"string", "default":"zdf,wdr,applaus,musik",
             "notes":"Comma-separated cleanup tokens removed in clean/markdown mode."},
            {"sml":"suffix", "type":"string", "default":"\"\"",
             "notes":"Optional text appended after processing."},
        ],
        "actions": [
            {"sms":"listen", "params":[], "returns":"void"},
            {"sms":"stop", "params":[], "returns":"void"},
            {"sms":"submitText", "params":[{"name":"text","type":"string"}], "returns":"void",
             "note":"Feeds raw text into the post-processing pipeline and emits result(...)."},
            {"sms":"setMode", "params":[{"name":"value","type":"string"}], "returns":"void"},
        ],
        "notes": [
            "Current vertical-slice behavior focuses on processing pipeline and emits result/rawResult signals.",
            "Android host integration uses a fixed singleton name: ForgeSpeechBridge.",
            "Bridge contract: startListening(objectId:int, language:string) -> bool, stopListening(objectId:int).",
            "Bridge callbacks into this control: _bridgeRawResult(text), _bridgeResult(text), _bridgeError(message).",
            "Use keyboard dictation as input source, then pass text through submitText() for Raw/Clean/Markdown processing."
        ],
        "examples_sml": [
            "SpeechRecognizer {",
            "    id: mic",
            "    language: \"de-DE\"",
            "    mode: clean",
            "    filters: \"zdf,wdr,applaus,musik\"",
            "}",
        ],
    }
