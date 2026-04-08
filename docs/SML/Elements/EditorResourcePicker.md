# EditorResourcePicker

## Inheritance

[EditorResourcePicker](EditorResourcePicker.md) → [HBoxContainer](HBoxContainer.md) → [BoxContainer](BoxContainer.md) → [Container](Container.md) → [Control](Control.md) → [CanvasItem](CanvasItem.md) → [Node](Node.md) → [Object](Object.md)

## Derived Classes

### Direct subclasses

- [EditorScriptPicker](EditorScriptPicker.md)

## Properties

This page lists **only properties declared by `EditorResourcePicker`**.
Inherited properties are documented in: [HBoxContainer](HBoxContainer.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|
| base_type | baseType | string | - |
| editable | editable | bool | - |
| toggle_mode | toggleMode | bool | - |

## Events

This page lists **only signals declared by `EditorResourcePicker`**.
Inherited signals are documented in: [HBoxContainer](HBoxContainer.md)

| Godot Signal | SMS Event | Params |
|-|-|-|
| resource_changed | `on <id>.resourceChanged(resource) { ... }` | Object resource |
| resource_selected | `on <id>.resourceSelected(resource, inspect) { ... }` | Object resource, bool inspect |

## Runtime Actions

This page lists **callable methods declared by `EditorResourcePicker`**.
Inherited actions are documented in: [HBoxContainer](HBoxContainer.md)

| Godot Method | SMS Call | Params | Returns |
|-|-|-|-|
| get_allowed_types | `<id>.getAllowedTypes()` | - | Variant |
| set_toggle_pressed | `<id>.setTogglePressed(pressed)` | bool pressed | void |
