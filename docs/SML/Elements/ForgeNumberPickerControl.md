# ForgeNumberPickerControl

## Inheritance

[ForgeNumberPickerControl](ForgeNumberPickerControl.md) → [LineEdit](LineEdit.md) → [Control](Control.md) → [CanvasItem](CanvasItem.md) → [Node](Node.md) → [Object](Object.md)

## Properties

This page lists **only properties declared by `ForgeNumberPickerControl`**.
Inherited properties are documented in: [LineEdit](LineEdit.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|

## Events

This page lists **only signals declared by `ForgeNumberPickerControl`**.
Inherited signals are documented in: [LineEdit](LineEdit.md)

| Godot Signal | SMS Event | Params |
|-|-|-|

## Runtime Actions

This page lists **callable methods declared by `ForgeNumberPickerControl`**.
Inherited actions are documented in: [LineEdit](LineEdit.md)

| Godot Method | SMS Call | Params | Returns |
|-|-|-|-|
| get_numeric_value | `<id>.getNumericValue()` | - | float |
| set_numeric_config | `<id>.setNumericConfig(axis, unit, color, step, dragSensitivity, decimals)` | string axis, string unit, Color color, float step, float dragSensitivity, int decimals | void |
| set_numeric_value | `<id>.setNumericValue(value)` | float value | void |

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

