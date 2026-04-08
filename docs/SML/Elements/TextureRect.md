# TextureRect

## Inheritance

[TextureRect](TextureRect.md) → [TextureRect](TextureRect.md) → [Control](Control.md) → [CanvasItem](CanvasItem.md) → [Node](Node.md) → [Object](Object.md)

## Derived Classes

### Direct subclasses

- [TextureRect](TextureRect.md)

## Properties

This page lists **only properties declared by `TextureRect`**.
Inherited properties are documented in: [Control](Control.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|
| - | id | identifier | - |
| - | src | string(url) | "" |
| - | alt | string | "" |
| - | width | int | - |
| - | height | int | - |
| - | shrinkH | bool | false |
| - | shrinkV | bool | false |

> Displays an image or SVG. Use width/height to set a fixed size, combined with shrinkH/shrinkV to prevent the control from expanding beyond that size inside a container.

### Examples

```sml
TextureRect {
    src: "res://logo.svg"
    width: 72
    height: 72
    shrinkH: true
    shrinkV: true
}
```

## Events

This page lists **only signals declared by `TextureRect`**.
Inherited signals are documented in: [Control](Control.md)

| Godot Signal | SMS Event | Params |
|-|-|-|

## Runtime Actions

This page lists **callable methods declared by `TextureRect`**.
Inherited actions are documented in: [Control](Control.md)

| Godot Method | SMS Call | Params | Returns |
|-|-|-|-|
| is_flipped_h | `<id>.isFlippedH()` | - | bool |
| is_flipped_v | `<id>.isFlippedV()` | - | bool |

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

