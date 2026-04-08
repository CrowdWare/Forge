# BaseButton

> Note: This is a base class included for inheritance documentation. It is **not** an SML element.

## Inheritance

[BaseButton](BaseButton.md) → [Control](Control.md) → [CanvasItem](CanvasItem.md) → [Node](Node.md) → [Object](Object.md)

## Derived Classes

### Direct subclasses

- [Button](Button.md)
- [LinkButton](LinkButton.md)
- [TextureButton](TextureButton.md)

## Properties

This page lists **only properties declared by `BaseButton`**.
Inherited properties are documented in: [Control](Control.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|
| action_mode | actionMode | int | - |
| button_mask | buttonMask | int | - |
| button_pressed | buttonPressed | bool | - |
| disabled | disabled | bool | - |
| keep_pressed_outside | keepPressedOutside | bool | - |
| shortcut_feedback | shortcutFeedback | bool | - |
| shortcut_in_tooltip | shortcutInTooltip | bool | - |
| toggle_mode | toggleMode | bool | - |

## Events

This page lists **only signals declared by `BaseButton`**.
Inherited signals are documented in: [Control](Control.md)

| Godot Signal | SMS Event | Params |
|-|-|-|
| button_down | `on <id>.buttonDown() { ... }` | - |
| button_up | `on <id>.buttonUp() { ... }` | - |
| pressed | `on <id>.pressed() { ... }` | - |
| toggled | `on <id>.toggled(toggledOn) { ... }` | bool toggledOn |

## Runtime Actions

This page lists **callable methods declared by `BaseButton`**.
Inherited actions are documented in: [Control](Control.md)

| Godot Method | SMS Call | Params | Returns |
|-|-|-|-|
| get_draw_mode | `<id>.getDrawMode()` | - | int |
| is_hovered | `<id>.isHovered()` | - | bool |
| is_pressed | `<id>.isPressed()` | - | bool |
| is_shortcut_in_tooltip_enabled | `<id>.isShortcutInTooltipEnabled()` | - | bool |
| set_pressed | `<id>.setPressed(pressed)` | bool pressed | void |
| set_pressed_no_signal | `<id>.setPressedNoSignal(pressed)` | bool pressed | void |

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

