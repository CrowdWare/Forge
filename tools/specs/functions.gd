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

# SMS Runtime Function Specification
# This file defines all built-in SMS functions in a structured format.

const FUNCTIONS := {
    "ui.createDialog": {
        "category": "UI",
        "signature": "ui.createDialog(path: String)",
        "description": "Creates a dialog from either a resource path (res:/...) or an inline SML definition string and returns the dialog instance."
    },

    "ui.createWindow": {
        "category": "UI",
        "signature": "ui.createWindow(smlText: String)",
        "description": "Creates a floating window from inline SML text and returns a Window instance (or null on failure). The returned Window supports runtime actions onClose(callbackName) and close()."
    },

    "getObject": {
        "category": "UI",
        "signature": "ui.getObject(name: String)",
        "description": "Returns a runtime object for any SML element with a matching id. Exposes callable methods dynamically from the underlying Godot object. Returns null when no matching id exists."
    },

    "ui.configureNumericLineEdit": {
        "category": "UI",
        "signature": "ui.configureNumericLineEdit(id: String, axis: String, unit: String, color: String, step: float, dragSensitivity: float, decimals: int)",
        "description": "Enables editor-like numeric behavior on a LineEdit: select-all on focus, drag-to-adjust while unfocused, and formatted preview text (for example: x 3.788 m)."
    },

    "ui.setNumericLineEditValue": {
        "category": "UI",
        "signature": "ui.setNumericLineEditValue(id: String, value: float)",
        "description": "Sets the numeric value of a configured numeric LineEdit and refreshes its displayed preview/raw text."
    },

    "ui.getNumericLineEditValue": {
        "category": "UI",
        "signature": "ui.getNumericLineEditValue(id: String)",
        "description": "Returns the current numeric value of a configured numeric LineEdit as a float."
    },

    "window.flags": {
        "category": "UI",
        "signature": "Window Flag Constants",
        "description": "Enum-like global constants for Window flags in SMS. Use with setFlag/getFlag without magic numbers, e.g. mainWindow.setFlag(extendToTitle, true). Available constants: borderless=0, alwaysOnTop=1, transparent=2, noFocus=3, popup=5, extendToTitle=6, mousePassthrough=7, sharpCorners=8, excludeFromCapture=9, popupWmHint=10, minSize=11, maxSize=12, resizeDisabled=13, transient=14, modal=15, popupExclusive=16."
    },

    "window.setFlag": {
        "category": "UI",
        "signature": "<windowId>.setFlag(flag: int, enabled: bool)",
        "description": "Sets a Window flag by enum constant (recommended) or integer value. Example: mainWindow.setFlag(extendToTitle, true)."
    },

    "window.getFlag": {
        "category": "UI",
        "signature": "<windowId>.getFlag(flag: int)",
        "description": "Gets a Window flag value by enum constant (recommended) or integer value. Example: var on = mainWindow.getFlag(extendToTitle)."
    },

    "ai.isConfigured": {
        "category": "AI",
        "signature": "ai.isConfigured()",
        "description": "Returns true when GROK_API_KEY is available to the runtime, otherwise false."
    },

    "ai.describeImage": {
        "category": "AI",
        "signature": "ai.describeImage(imagePath: String, prompt: String?, model: String?)",
        "description": "Sends a local image to Grok Vision and returns the textual analysis. imagePath may be absolute or relative to the app project root."
    },

    "ai.stylizeImage": {
        "category": "AI",
        "signature": "ai.stylizeImage(posePath: String, outputPath: String, prompt: String, stylePath: String?, extraPath: String?, negativePrompt: String?, model: String?)",
        "description": "Runs image-to-image stylization via Grok using pose/style/extra references and writes the generated image to outputPath. Returns the resolved output path (supports <version> placeholder)."
    },

    "ai.stylizeVideo": {
        "category": "AI",
        "signature": "ai.stylizeVideo(inputVideoPath: String, outputPath: String, prompt: String, negativePrompt: String?, model: String?)",
        "description": "Runs video-to-video stylization via Grok, polls job completion, downloads the styled video, and returns the resolved output path (supports <version> placeholder)."
    },

    "ai.createVideoFromFrames": {
        "category": "AI",
        "signature": "ai.createVideoFromFrames(framesDirectory: String, fps: int, outputPath: String, pattern: String?)",
        "description": "Encodes a PNG frame sequence to MP4 via ffmpeg. Returns outputPath on success or empty string on failure. Default pattern is frame_%04d.png."
    },

    "log.success": {
        "category": "Logging",
        "signature": "log.success(message: String)",
        "description": "Logs a success message."
    },

    "log.info": {
        "category": "Logging",
        "signature": "log.info(message: String)",
        "description": "Logs an informational message."
    },

    "log.warn": {
        "category": "Logging",
        "signature": "log.warn(message: String)",
        "description": "Logs a warning message."
    },

    "log.error": {
        "category": "Logging",
        "signature": "log.error(message: String)",
        "description": "Logs an error message."
    },
    "log.debug": {
        "category": "Logging",
        "signature": "log.debug(message: String)",
        "description": "Logs a debug message when debug logging is enabled in startup settings."
    },

    "fs.list": {
        "category": "File System",
        "signature": "fs.list(path: String)",
        "description": "Lists files in a directory and returns an array of file names. Path must use res:/, appRes:/, or user:/ (single slash, no '..' traversal segments). When a host sandbox callback is registered, host trust policy is enforced."
    },

    "fs.writeText": {
        "category": "File System",
        "signature": "fs.writeText(path: String, content: String)",
        "description": "Writes text content to a file. Path must use res:/, appRes:/, or user:/ (single slash, no '..' traversal segments). When a host sandbox callback is registered, host trust policy is enforced."
    },

    "fs.readText": {
        "category": "File System",
        "signature": "fs.readText(path: String)",
        "description": "Reads text content from a file and returns it as a String. Path must use res:/, appRes:/, or user:/ (single slash, no '..' traversal segments). When a host sandbox callback is registered, host trust policy is enforced."
    },

    "i18n.tr": {
        "category": "Internationalization",
        "signature": "i18n.tr(key: String, default: String?)",
        "description": "Returns the translation for the given key in the current locale. Falls back to the default value when provided, or to the key itself when no default is given and no translation is found."
    },

    "i18n.getLocale": {
        "category": "Internationalization",
        "signature": "i18n.getLocale()",
        "description": "Returns the active two-letter ISO language code (for example: en, de, es)."
    },

    "i18n.setLocale": {
        "category": "Internationalization",
        "signature": "i18n.setLocale(locale: String)",
        "description": "Switches the active locale and reloads the corresponding strings-<locale>.sml file. Subsequent i18n.tr() calls return translations for the new locale. Controls whose text was already rendered during UI build are not automatically updated; update them explicitly via script after calling setLocale. The call is a no-op when the requested locale is already active. A warning is logged on load failure and the previous locale is retained."
    },

    "os.getLocale": {
        "category": "OS",
        "signature": "os.getLocale()",
        "description": "Returns the current locale in language_COUNTRY format (for example: de_DE)."
    },

    "os.getLanguage": {
        "category": "OS",
        "signature": "os.getLanguage()",
        "description": "Returns the current two-letter ISO language code (for example: de)."
    },

    "os.getCountry": {
        "category": "OS",
        "signature": "os.getCountry()",
        "description": "Returns the current two-letter ISO country code (for example: DE)."
    },

    "os.getTimeZone": {
        "category": "OS",
        "signature": "os.getTimeZone()",
        "description": "Returns the local time zone identifier (for example: Europe/Berlin)."
    },

    "os.getPlatform": {
        "category": "OS",
        "signature": "os.getPlatform()",
        "description": "Returns the current platform name: mac, linux, windows, or android."
    },

    "os.getArch": {
        "category": "OS",
        "signature": "os.getArch()",
        "description": "Returns the current process architecture (for example: arm64 or x64)."
    },

    "os.isMobile": {
        "category": "OS",
        "signature": "os.isMobile()",
        "description": "Returns true when running on a mobile platform (currently android), otherwise false."
    },

    "os.isDesktop": {
        "category": "OS",
        "signature": "os.isDesktop()",
        "description": "Returns true when running on a desktop platform, otherwise false."
    },

    "os.now": {
        "category": "OS",
        "signature": "os.now()",
        "description": "Returns the current Unix epoch time in milliseconds."
    },

    "os.getUptime": {
        "category": "OS",
        "signature": "os.getUptime()",
        "description": "Returns seconds since SMS runtime initialization."
    },

    "os.getEnv": {
        "category": "OS",
        "signature": "os.getEnv(name: String)",
        "description": "Returns the process environment variable value or empty string if unset."
    },

    "os.setEnv": {
        "category": "OS",
        "signature": "os.setEnv(name: String, value: String)",
        "description": "Sets a process-level environment variable for the running app session. Not persisted system-wide."
    },

    "os.resolvePath": {
        "category": "OS",
        "signature": "os.resolvePath(path: String)",
        "description": "Resolves res:/, user:/ and relative paths to an absolute filesystem path."
    },

    "os.callStatic": {
        "category": "OS",
        "signature": "os.callStatic(assemblyPath: String, typeName: String, methodName: String, ...args)",
        "description": "Loads a .NET assembly and invokes a static method by reflection. Returns the method result converted to an SMS value."
    },

    "os.smsRuntimeMode": {
        "category": "OS",
        "signature": "os.smsRuntimeMode()",
        "description": "Returns the currently active SMS execution mode as String: interpreter or native."
    },

    "os.runtimeMode": {
        "category": "OS",
        "signature": "os.runtimeMode()",
        "description": "Alias for os.smsRuntimeMode(). Returns interpreter or native."
    }
}

# CHANGE: This file is NOT an SML element spec. It exists for SMS function docs.
# The SML element docs generator scans tools/specs/*.gd and expects get_spec().
# Return an empty spec so it can ignore this file without errors.
static func get_spec() -> Dictionary: # CHANGE
    return {
        "name": "sms_functions",
        "elements": []
    }
