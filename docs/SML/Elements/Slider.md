# Slider

> Note: This is a base class included for inheritance documentation. It is **not** an SML element.

## Inheritance

[Slider](Slider.md) → [Range](Range.md) → [Control](Control.md) → [CanvasItem](CanvasItem.md) → [Node](Node.md) → [Object](Object.md)

## Derived Classes

### Direct subclasses

- [HSlider](HSlider.md)
- [VSlider](VSlider.md)

## Properties

This page lists **only properties declared by `Slider`**.
Inherited properties are documented in: [Range](Range.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|
| editable | editable | bool | - |
| scrollable | scrollable | bool | - |
| tick_count | tickCount | int | - |
| ticks_on_borders | ticksOnBorders | bool | - |
| ticks_position | ticksPosition | int | - |

## Events

This page lists **only signals declared by `Slider`**.
Inherited signals are documented in: [Range](Range.md)

| Godot Signal | SMS Event | Params |
|-|-|-|
| drag_ended | `on <id>.dragEnded(valueChanged) { ... }` | bool valueChanged |
| drag_started | `on <id>.dragStarted() { ... }` | - |

## Runtime Actions

This page lists **callable methods declared by `Slider`**.
Inherited actions are documented in: [Range](Range.md)

| Godot Method | SMS Call | Params | Returns |
|-|-|-|-|
| get_ticks | `<id>.getTicks()` | - | int |
| set_ticks | `<id>.setTicks(count)` | int count | void |

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

