# SplitContainer

## Inheritance

[SplitContainer](SplitContainer.md) → [Container](Container.md) → [Control](Control.md) → [CanvasItem](CanvasItem.md) → [Node](Node.md) → [Object](Object.md)

## Derived Classes

### Direct subclasses

- [HSplitContainer](HSplitContainer.md)
- [VSplitContainer](VSplitContainer.md)

## Properties

This page lists **only properties declared by `SplitContainer`**.
Inherited properties are documented in: [Container](Container.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|
| collapsed | collapsed | bool | - |
| drag_area_highlight_in_editor | dragAreaHighlightInEditor | bool | - |
| drag_area_margin_begin | dragAreaMarginBegin | int | - |
| drag_area_margin_end | dragAreaMarginEnd | int | - |
| drag_area_offset | dragAreaOffset | int | - |
| dragger_visibility | draggerVisibility | int | - |
| dragging_enabled | draggingEnabled | bool | - |
| touch_dragger_enabled | touchDraggerEnabled | bool | - |
| vertical | vertical | bool | - |

## Events

This page lists **only signals declared by `SplitContainer`**.
Inherited signals are documented in: [Container](Container.md)

| Godot Signal | SMS Event | Params |
|-|-|-|
| drag_ended | `on <id>.dragEnded() { ... }` | - |
| drag_started | `on <id>.dragStarted() { ... }` | - |
| dragged | `on <id>.dragged(offset) { ... }` | int offset |

## Runtime Actions

This page lists **callable methods declared by `SplitContainer`**.
Inherited actions are documented in: [Container](Container.md)

| Godot Method | SMS Call | Params | Returns |
|-|-|-|-|
| clamp_split_offset | `<id>.clampSplitOffset(priorityIndex)` | int priorityIndex | void |
| get_drag_area_control | `<id>.getDragAreaControl()` | - | Object |
| get_drag_area_controls | `<id>.getDragAreaControls()` | - | Variant |
| is_drag_area_highlight_in_editor_enabled | `<id>.isDragAreaHighlightInEditorEnabled()` | - | bool |

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

