# ColorPickerButton

## Inheritance

[ColorPickerButton](ColorPickerButton.md) → [Button](Button.md) → [BaseButton](BaseButton.md) → [Control](Control.md) → [CanvasItem](CanvasItem.md) → [Node](Node.md) → [Object](Object.md)

## Properties

This page lists **only properties declared by `ColorPickerButton`**.
Inherited properties are documented in: [Button](Button.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|
| color | color | Color | - |
| edit_alpha | editAlpha | bool | - |
| edit_intensity | editIntensity | bool | - |

## Events

This page lists **only signals declared by `ColorPickerButton`**.
Inherited signals are documented in: [Button](Button.md)

| Godot Signal | SMS Event | Params |
|-|-|-|
| color_changed | `on <id>.colorChanged(color) { ... }` | Color color |
| picker_created | `on <id>.pickerCreated() { ... }` | - |
| popup_closed | `on <id>.popupClosed() { ... }` | - |

## Runtime Actions

This page lists **callable methods declared by `ColorPickerButton`**.
Inherited actions are documented in: [Button](Button.md)

| Godot Method | SMS Call | Params | Returns |
|-|-|-|-|
| get_pick_color | `<id>.getPickColor()` | - | Color |
| get_picker | `<id>.getPicker()` | - | Object |
| get_popup | `<id>.getPopup()` | - | Object |
| is_editing_alpha | `<id>.isEditingAlpha()` | - | bool |
| is_editing_intensity | `<id>.isEditingIntensity()` | - | bool |
| set_pick_color | `<id>.setPickColor(color)` | Color color | void |

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

