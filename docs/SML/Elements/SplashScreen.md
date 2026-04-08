# SplashScreen

## Inheritance

[SplashScreen](SplashScreen.md) → [Panel](Panel.md) → [Control](Control.md) → [CanvasItem](CanvasItem.md) → [Node](Node.md) → [Object](Object.md)

## Properties

This page lists **only properties declared by `SplashScreen`**.
Inherited properties are documented in: [Panel](Panel.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|
| - | id | identifier | - |
| - | title | string | "" |
| - | size | vec2i | 640, 480 |
| - | pos | vec2i | 0, 0 |
| - | minSize | vec2i | 0, 0 |
| - | duration | int | 0 |
| - | loadOnReady | string(url) | "" |

> Startup screen shown before the main app loads. Shown immediately after entry files are downloaded. Remaining assets load in background with an optional ProgressBar child.
> SplashScreen is always centered on screen and uses extendToTitle so the content fills the title bar area while the system close button remains visible. These behaviours are implicit - no properties needed.

### Examples

```sml
SplashScreen {
    id: splash
    size: 640, 480
    duration: 3000
    loadOnReady: "res://docs/Default/main.sml"

    Label { text: "Loading..." }
    ProgressBar {
        id: downloadProgress
        showPercentage: false
        visible: false
    }
}
```

## Events

This page lists **only signals declared by `SplashScreen`**.
Inherited signals are documented in: [Panel](Panel.md)

| Godot Signal | SMS Event | Params |
|-|-|-|
