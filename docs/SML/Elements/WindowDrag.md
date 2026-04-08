# WindowDrag

## Inheritance

[WindowDrag](WindowDrag.md) → [Panel](Panel.md) → [Control](Control.md) → [CanvasItem](CanvasItem.md) → [Node](Node.md) → [Object](Object.md)

## Properties

This page lists **only properties declared by `WindowDrag`**.
Inherited properties are documented in: [Panel](Panel.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|
| - | id | identifier | - |
| - | anchors | string | "" |
| - | x | int | 0 |
| - | y | int | 0 |
| - | width | int | 0 |
| - | height | int | 32 |

> Provides a native draggable title/caption area for custom frameless layouts.
> Single left click starts OS window drag.
> Double left click toggles maximize/restore (windowed <-> maximized).
> Useful together with Window.extendToTitle: true.
> When used with DockingHost, set WindowDrag.height analogous to DockingHost.offsetTop so caption drag area and dock top offset align.

### Examples

```sml
Window {
    id: mainWindow
    extendToTitle: true

    WindowDrag {
        id: titleDrag
        anchors: left | top | right
        height: 34
    }
}
```

## Events

This page lists **only signals declared by `WindowDrag`**.
Inherited signals are documented in: [Panel](Panel.md)

| Godot Signal | SMS Event | Params |
|-|-|-|
