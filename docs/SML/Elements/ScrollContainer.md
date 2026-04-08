# ScrollContainer

## Inheritance

[ScrollContainer](ScrollContainer.md) → [Container](Container.md) → [Control](Control.md) → [CanvasItem](CanvasItem.md) → [Node](Node.md) → [Object](Object.md)

## Derived Classes

### Direct subclasses

- [EditorInspector](EditorInspector.md)

## Properties

This page lists **only properties declared by `ScrollContainer`**.
Inherited properties are documented in: [Container](Container.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|
| draw_focus_border | drawFocusBorder | bool | - |
| follow_focus | followFocus | bool | - |
| horizontal_scroll_mode | horizontalScrollMode | int | - |
| scroll_deadzone | scrollDeadzone | int | - |
| scroll_hint_mode | scrollHintMode | int | - |
| scroll_horizontal | scrollHorizontal | int | - |
| scroll_horizontal_custom_step | scrollHorizontalCustomStep | float | - |
| scroll_vertical | scrollVertical | int | - |
| scroll_vertical_custom_step | scrollVerticalCustomStep | float | - |
| tile_scroll_hint | tileScrollHint | bool | - |
| vertical_scroll_mode | verticalScrollMode | int | - |

## Events

This page lists **only signals declared by `ScrollContainer`**.
Inherited signals are documented in: [Container](Container.md)

| Godot Signal | SMS Event | Params |
|-|-|-|
| scroll_ended | `on <id>.scrollEnded() { ... }` | - |
| scroll_started | `on <id>.scrollStarted() { ... }` | - |

## Runtime Actions

This page lists **callable methods declared by `ScrollContainer`**.
Inherited actions are documented in: [Container](Container.md)

| Godot Method | SMS Call | Params | Returns |
|-|-|-|-|
| get_deadzone | `<id>.getDeadzone()` | - | int |
| get_h_scroll | `<id>.getHScroll()` | - | int |
| get_h_scroll_bar | `<id>.getHScrollBar()` | - | Object |
| get_horizontal_custom_step | `<id>.getHorizontalCustomStep()` | - | float |
| get_v_scroll | `<id>.getVScroll()` | - | int |
| get_v_scroll_bar | `<id>.getVScrollBar()` | - | Object |
| get_vertical_custom_step | `<id>.getVerticalCustomStep()` | - | float |
| is_following_focus | `<id>.isFollowingFocus()` | - | bool |
| is_scroll_hint_tiled | `<id>.isScrollHintTiled()` | - | bool |
| set_deadzone | `<id>.setDeadzone(deadzone)` | int deadzone | void |
| set_h_scroll | `<id>.setHScroll(value)` | int value | void |
| set_horizontal_custom_step | `<id>.setHorizontalCustomStep(value)` | float value | void |
| set_v_scroll | `<id>.setVScroll(value)` | int value | void |
| set_vertical_custom_step | `<id>.setVerticalCustomStep(value)` | float value | void |

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

