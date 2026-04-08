# ScriptEditor

## Inheritance

[ScriptEditor](ScriptEditor.md) → [PanelContainer](PanelContainer.md) → [Container](Container.md) → [Control](Control.md) → [CanvasItem](CanvasItem.md) → [Node](Node.md) → [Object](Object.md)

## Properties

This page lists **only properties declared by `ScriptEditor`**.
Inherited properties are documented in: [PanelContainer](PanelContainer.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|

## Events

This page lists **only signals declared by `ScriptEditor`**.
Inherited signals are documented in: [PanelContainer](PanelContainer.md)

| Godot Signal | SMS Event | Params |
|-|-|-|
| editor_script_changed | `on <id>.editorScriptChanged(script) { ... }` | Object script |
| script_close | `on <id>.scriptClose(script) { ... }` | Object script |

## Runtime Actions

This page lists **callable methods declared by `ScriptEditor`**.
Inherited actions are documented in: [PanelContainer](PanelContainer.md)

| Godot Method | SMS Call | Params | Returns |
|-|-|-|-|
| get_breakpoints | `<id>.getBreakpoints()` | - | Variant |
| get_current_editor | `<id>.getCurrentEditor()` | - | Object |
| get_current_script | `<id>.getCurrentScript()` | - | Object |
| get_open_script_editors | `<id>.getOpenScriptEditors()` | - | Variant |
| get_open_scripts | `<id>.getOpenScripts()` | - | Variant |
| goto_help | `<id>.gotoHelp(topic)` | string topic | void |
| goto_line | `<id>.gotoLine(lineNumber)` | int lineNumber | void |
| open_script_create_dialog | `<id>.openScriptCreateDialog(baseName, basePath)` | string baseName, string basePath | void |

## Attached Properties

These properties are declared by a parent provider and set on this element using the qualified syntax `<providerId>.property: value` or `ProviderType.property: value`.

### Provided by `TabContainer`

| Attached Property | Type | Description |
|-|-|-|
| title | string | Tab title read by the parent TabContainer. Use attached property syntax: `<containerId>.title: "Caption"` or `TabContainer.title: "Caption"`. |

### Provided by `DockingContainer`

| Attached Property | Type | Description |
|-|-|-|
| title | string | Tab title read by the parent DockingContainer. Use attached property syntax: `<containerId>.title: "Caption"` or `DockingContainer.title: "Caption"`. |

