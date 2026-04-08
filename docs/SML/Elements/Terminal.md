# Terminal

## Inheritance

[Terminal](Terminal.md) → [Panel](Panel.md) → [Control](Control.md) → [CanvasItem](CanvasItem.md) → [Node](Node.md) → [Object](Object.md)

## Properties

This page lists **only properties declared by `Terminal`**.
Inherited properties are documented in: [Panel](Panel.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|
| - | id | identifier | - |
| - | title | string | "" |
| - | size | vec2i | 640, 480 |
| - | pos | vec2i | 0, 0 |

> Headless/CLI-oriented root marker.
> ForgeRunner uses this root to route startup download progress to console/log output.

### Examples

```sml
Terminal {
    id: demoTerminal
}
```

## Events

This page lists **only signals declared by `Terminal`**.
Inherited signals are documented in: [Panel](Panel.md)

| Godot Signal | SMS Event | Params |
|-|-|-|
