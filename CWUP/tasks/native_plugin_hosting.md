# Native Plugin Hosting

## Status
- [ ] Postponed / Deferred (2026-03-10): Plugin capabilities are deprioritized in favor of command-first CLI workflows.

## Goal
Port the runtime plugin system from C# (`Main.cs`, lines 677–992) to C++ so that
`ForgeRunner.Native` can discover, load, and host SML/SMS-based plugins from a
`plugins/` subdirectory - and call into native shared libraries instead of .NET
assemblies.

## Context
The C# runner already has a complete, production-proven plugin system used by
`ForgePoser` (PromptPlugin, ExportGlbPlugin). The architecture is:

1. **Discovery** - scan `{appDir}/plugins/*/plugin.sml` at startup.
2. **Panel build** - each plugin may provide a `panelSml:` file → built into a
   `Control` and attached as a tab in the target `ForgeDockingContainerControl`.
3. **Script load** - each plugin may provide an `sms:` file → loaded into the
   **same** SMS engine session as the main app (shared namespace).
4. **Native extension** - each plugin may provide an `assemblyPath:` pointing to a
   .NET DLL; called via `os.callStatic()` using reflection.
   In native C++ this becomes a shared library (`.so` / `.dylib` / `.dll`) with a
   plain C ABI, called via `dlopen` + `dlsym` / `LoadLibrary` + `GetProcAddress`.

## Subsystems to Port

### 1 - Plugin Descriptor

```cpp
struct PluginDescriptor {
    std::string id;
    std::string title;       // defaults to id
    std::string dock_side;   // "left", "leftBottom", "center", etc.
    std::string panel_sml;   // absolute path to panel SML (optional)
    std::string sms_path;    // absolute path to SMS script (optional)
    std::string lib_path;    // absolute path to shared library (optional)
    bool enabled = true;
};
```

### 2 - Discovery (`discover_plugins`)
Scan `{appDir}/plugins/` for subdirectories each containing `plugin.sml`.
Parse each descriptor via `smlcore::parse_document()`. Skip disabled or
duplicate-id entries.

### 3 - Panel build (`build_plugin_panel`)
Re-use `forge::UiBuilder` (already implemented) to build the `panelSml` file
into a `Control*`. Set the control name to `{pluginId}Panel`.

### 4 - Dock attachment (`attach_plugin_panel`)
Find the `ForgeDockingContainerControl` matching the requested `dock_side` (by
comparing `dock_side_` member). Fall back to `dockrightbottom` if not found.
Call the existing `add_tab` / `add_child` mechanism on the container.
Set the tab title via meta (`{containerId}.title`).

### 5 - SMS script loading
After the main SMS session is created (via `SMSCore.Native`), call
`sms_session_load_additional(session_id, sms_source, uri, …)` - or an equivalent
extended API if the current C ABI does not expose multi-script loading.

The plugin SMS runs in the **same session** as the main app, so it can call
`ui.getObject("editor")` etc.

### 6 - Native plugin library (`os.callPlugin`)
Replace .NET's `os.callStatic(assemblyPath, typeName, methodName, args)` with a
native equivalent `os.callPlugin(libPath, functionName, args)` registered in the
SMS session.

Expected C ABI for a native plugin function:
```c
// All args passed as a JSON array string; return value as JSON string.
const char* forge_plugin_call(const char* function_name,
                              const char* args_json,
                              char* out_error, int out_error_size);
```

Loading:
```cpp
void* handle = dlopen(lib_path.c_str(), RTLD_LAZY | RTLD_LOCAL);
auto fn = (ForgePluginCallFn)dlsym(handle, "forge_plugin_call");
```

The function signature is fixed (no per-method symbol lookup) - `function_name`
is passed as an argument so one entry point covers all plugin methods.

### 7 - Lifecycle
```
ForgeRunnerNativeMain::show_sml(path)
    └── (after UI built, DockingHost confirmed)
        load_plugins(appDir, session_id, dock_host)
            ├── discover_plugins(appDir)
            ├── for each descriptor:
            │   ├── build_plugin_panel(panel_sml) → Control*
            │   ├── attach_plugin_panel(dock_host, dock_side, panel)
            │   └── load_plugin_sms(session_id, sms_path)
            └── (SMS InvokeReady called after all plugins loaded)
```

### 8 - Duplicate ID Prevention
Track loaded plugin ids in `std::unordered_set<std::string> loaded_plugin_ids_`.
Skip any plugin whose id is already present.

## Plugin Spec (tools/specs)
Add `tools/specs/plugin.gd` with:
```gdscript
{
    "name": "Plugin",
    "backing_native": "-",  # not a Control; descriptor-only
    "properties": [
        {"sml":"id",          "type":"identifier"},
        {"sml":"enabled",     "type":"bool",   "default":"true"},
        {"sml":"title",       "type":"string", "default":"-"},
        {"sml":"dock",        "type":"enum",   "default":"dockRightBottom"},
        {"sml":"panelSml",    "type":"string", "default":"-"},
        {"sml":"sms",         "type":"string", "default":"-"},
        {"sml":"assemblyPath","type":"string", "default":"-"},
    ],
}
```

## Native Plugin C ABI Contract
Document the expected interface that a native plugin shared library must export,
so third-party plugins can be built against a stable ABI.

## Acceptance Criteria
- `ForgePoser` loads both existing plugins (PromptPlugin, ExportGlbPlugin) when
  running under `ForgeRunner.Native` - panels appear in the expected dock slots.
- `os.callPlugin(libPath, "SetApiKey", argsJson)` calls into a `.dylib`/`.so`.
- Plugin SMS shares the main app SMS session (can call `ui.getObject("editor")`).
- Duplicate plugin IDs are silently skipped.
- Missing/invalid `plugin.sml` files produce a warning, not a crash.

## Priority
Implement after `native_sms_ui_bridge.md` (SMS bridge is a prerequisite for
plugin SMS loading to work).

## Reference
- C#: `ForgeRunner/Main.cs` lines 677–992
- C#: `ForgeRunner/Runtime/UI/SmsUiRuntime.cs` (`LoadAdditionalScriptFromUriAsync`,
  `os.callStatic`)
- Examples: `ForgePoser/plugins/prompt/`, `ForgePoser/plugins/export_glb/`
