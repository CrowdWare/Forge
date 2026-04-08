# ScriptEditorBase

## Inheritance

[ScriptEditorBase](ScriptEditorBase.md) → [VBoxContainer](VBoxContainer.md) → [BoxContainer](BoxContainer.md) → [Container](Container.md) → [Control](Control.md) → [CanvasItem](CanvasItem.md) → [Node](Node.md) → [Object](Object.md)

## Properties

This page lists **only properties declared by `ScriptEditorBase`**.
Inherited properties are documented in: [VBoxContainer](VBoxContainer.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|

## Events

This page lists **only signals declared by `ScriptEditorBase`**.
Inherited signals are documented in: [VBoxContainer](VBoxContainer.md)

| Godot Signal | SMS Event | Params |
|-|-|-|
| edited_script_changed | `on <id>.editedScriptChanged() { ... }` | - |
| go_to_help | `on <id>.goToHelp(what) { ... }` | string what |
| go_to_method | `on <id>.goToMethod(script, method) { ... }` | Object script, string method |
| name_changed | `on <id>.nameChanged() { ... }` | - |
| replace_in_files_requested | `on <id>.replaceInFilesRequested(text) { ... }` | string text |
| request_help | `on <id>.requestHelp(topic) { ... }` | string topic |
| request_open_script_at_line | `on <id>.requestOpenScriptAtLine(script, line) { ... }` | Object script, int line |
| request_save_history | `on <id>.requestSaveHistory() { ... }` | - |
| request_save_previous_state | `on <id>.requestSavePreviousState(state) { ... }` | Variant state |
| search_in_files_requested | `on <id>.searchInFilesRequested(text) { ... }` | string text |

## Runtime Actions

This page lists **callable methods declared by `ScriptEditorBase`**.
Inherited actions are documented in: [VBoxContainer](VBoxContainer.md)

| Godot Method | SMS Call | Params | Returns |
|-|-|-|-|
| get_base_editor | `<id>.getBaseEditor()` | - | Object |

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

