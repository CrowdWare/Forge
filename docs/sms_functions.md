# SMS Runtime Functions

This document lists built-in SMS runtime helper functions available in Forge.

## AI

### ai.createVideoFromFrames(framesDirectory: String, fps: int, outputPath: String, pattern: String?)
Encodes a PNG frame sequence to MP4 via ffmpeg. Returns outputPath on success or empty string on failure. Default pattern is frame_%04d.png.

### ai.describeImage(imagePath: String, prompt: String?, model: String?)
Sends a local image to Grok Vision and returns the textual analysis. imagePath may be absolute or relative to the app project root.

### ai.isConfigured()
Returns true when GROK_API_KEY is available to the runtime, otherwise false.

### ai.stylizeImage(posePath: String, outputPath: String, prompt: String, stylePath: String?, extraPath: String?, negativePrompt: String?, model: String?)
Runs image-to-image stylization via Grok using pose/style/extra references and writes the generated image to outputPath. Returns the resolved output path (supports <version> placeholder).

### ai.stylizeVideo(inputVideoPath: String, outputPath: String, prompt: String, negativePrompt: String?, model: String?)
Runs video-to-video stylization via Grok, polls job completion, downloads the styled video, and returns the resolved output path (supports <version> placeholder).

## File System

### fs.list(path: String)
Lists files in a directory and returns an array of file names. Path must use res:/, appRes:/, or user:/ (single slash, no '..' traversal segments). When a host sandbox callback is registered, host trust policy is enforced.

### fs.readText(path: String)
Reads text content from a file and returns it as a String. Path must use res:/, appRes:/, or user:/ (single slash, no '..' traversal segments). When a host sandbox callback is registered, host trust policy is enforced.

### fs.writeText(path: String, content: String)
Writes text content to a file. Path must use res:/, appRes:/, or user:/ (single slash, no '..' traversal segments). When a host sandbox callback is registered, host trust policy is enforced.

## Internationalization

### i18n.getLocale()
Returns the active two-letter ISO language code (for example: en, de, es).

### i18n.setLocale(locale: String)
Switches the active locale and reloads the corresponding strings-<locale>.sml file. Subsequent i18n.tr() calls return translations for the new locale. Controls whose text was already rendered during UI build are not automatically updated; update them explicitly via script after calling setLocale. The call is a no-op when the requested locale is already active. A warning is logged on load failure and the previous locale is retained.

### i18n.tr(key: String, default: String?)
Returns the translation for the given key in the current locale. Falls back to the default value when provided, or to the key itself when no default is given and no translation is found.

## Logging

### log.debug(message: String)
Logs a debug message when debug logging is enabled in startup settings.

### log.error(message: String)
Logs an error message.

### log.info(message: String)
Logs an informational message.

### log.success(message: String)
Logs a success message.

### log.warn(message: String)
Logs a warning message.

## OS

### os.callStatic(assemblyPath: String, typeName: String, methodName: String, ...args)
Loads a .NET assembly and invokes a static method by reflection. Returns the method result converted to an SMS value.

### os.getArch()
Returns the current process architecture (for example: arm64 or x64).

### os.getCountry()
Returns the current two-letter ISO country code (for example: DE).

### os.getEnv(name: String)
Returns the process environment variable value or empty string if unset.

### os.getLanguage()
Returns the current two-letter ISO language code (for example: de).

### os.getLocale()
Returns the current locale in language_COUNTRY format (for example: de_DE).

### os.getPlatform()
Returns the current platform name: mac, linux, windows, or android.

### os.getTimeZone()
Returns the local time zone identifier (for example: Europe/Berlin).

### os.getUptime()
Returns seconds since SMS runtime initialization.

### os.isDesktop()
Returns true when running on a desktop platform, otherwise false.

### os.isMobile()
Returns true when running on a mobile platform (currently android), otherwise false.

### os.now()
Returns the current Unix epoch time in milliseconds.

### os.resolvePath(path: String)
Resolves res:/, user:/ and relative paths to an absolute filesystem path.

### os.runtimeMode()
Alias for os.smsRuntimeMode(). Returns interpreter or native.

### os.setEnv(name: String, value: String)
Sets a process-level environment variable for the running app session. Not persisted system-wide.

### os.smsRuntimeMode()
Returns the currently active SMS execution mode as String: interpreter or native.

## UI

### ui.getObject(name: String)
Returns a runtime object for any SML element with a matching id. Exposes callable methods dynamically from the underlying Godot object. Returns null when no matching id exists.

### ui.configureNumericLineEdit(id: String, axis: String, unit: String, color: String, step: float, dragSensitivity: float, decimals: int)
Enables editor-like numeric behavior on a LineEdit: select-all on focus, drag-to-adjust while unfocused, and formatted preview text (for example: x 3.788 m).

### ui.createDialog(path: String)
Creates a dialog from either a resource path (res:/...) or an inline SML definition string and returns the dialog instance.

### ui.createWindow(smlText: String)
Creates a floating window from inline SML text and returns a Window instance (or null on failure). The returned Window supports runtime actions onClose(callbackName) and close().

### ui.getNumericLineEditValue(id: String)
Returns the current numeric value of a configured numeric LineEdit as a float.

### ui.setNumericLineEditValue(id: String, value: float)
Sets the numeric value of a configured numeric LineEdit and refreshes its displayed preview/raw text.

### ui.setTheme(mode: String)
Switches the active color theme at runtime by loading `theme.{mode}.sml` (e.g. `theme.dark.sml` or `theme.light.sml`) from the app directory and re-applying all `@Colors.*` token bindings to the live control tree. Controls whose color properties were declared with `@Colors.*` references are updated automatically; controls with hard-coded color values are unaffected.

### ui.animate(id: String, property: String, toValue, durationMs: Int)
Animates a UI element property using Godot's Tween. Supported properties: `"scale"` (pulse: scales up to `toValue`, then back to 1.0), `"modulate"` (color tween to the hex string `toValue`). `durationMs` is the duration of each tween step in milliseconds.

### Window Flag Constants
Enum-like global constants for Window flags in SMS. Use with setFlag/getFlag without magic numbers, e.g. mainWindow.setFlag(extendToTitle, true). Available constants: borderless=0, alwaysOnTop=1, transparent=2, noFocus=3, popup=5, extendToTitle=6, mousePassthrough=7, sharpCorners=8, excludeFromCapture=9, popupWmHint=10, minSize=11, maxSize=12, resizeDisabled=13, transient=14, modal=15, popupExclusive=16.

### <windowId>.getFlag(flag: int)
Gets a Window flag value by enum constant (recommended) or integer value. Example: var on = mainWindow.getFlag(extendToTitle).

### <windowId>.setFlag(flag: int, enabled: bool)
Sets a Window flag by enum constant (recommended) or integer value. Example: mainWindow.setFlag(extendToTitle, true).

