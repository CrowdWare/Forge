# SpinBox

## Inheritance

[SpinBox](SpinBox.md) → [Range](Range.md) → [Control](Control.md) → [CanvasItem](CanvasItem.md) → [Node](Node.md) → [Object](Object.md)

## Properties

This page lists **only properties declared by `SpinBox`**.
Inherited properties are documented in: [Range](Range.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|
| alignment | alignment | int | - |
| custom_arrow_round | customArrowRound | bool | - |
| custom_arrow_step | customArrowStep | float | - |
| editable | editable | bool | - |
| prefix | prefix | string | - |
| select_all_on_focus | selectAllOnFocus | bool | - |
| suffix | suffix | string | - |
| update_on_text_changed | updateOnTextChanged | bool | - |

## Events

This page lists **only signals declared by `SpinBox`**.
Inherited signals are documented in: [Range](Range.md)

| Godot Signal | SMS Event | Params |
|-|-|-|

## Runtime Actions

This page lists **callable methods declared by `SpinBox`**.
Inherited actions are documented in: [Range](Range.md)

| Godot Method | SMS Call | Params | Returns |
|-|-|-|-|
| apply | `<id>.apply()` | - | void |
| get_horizontal_alignment | `<id>.getHorizontalAlignment()` | - | int |
| get_line_edit | `<id>.getLineEdit()` | - | Object |
| is_custom_arrow_rounding | `<id>.isCustomArrowRounding()` | - | bool |
| set_horizontal_alignment | `<id>.setHorizontalAlignment(alignment)` | int alignment | void |

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

