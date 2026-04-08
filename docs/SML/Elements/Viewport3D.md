# Viewport3D

## Inheritance

[Viewport3D](Viewport3D.md) → [SubViewportContainer](SubViewportContainer.md) → [Container](Container.md) → [Control](Control.md) → [CanvasItem](CanvasItem.md) → [Node](Node.md) → [Object](Object.md)

## Properties

This page lists **only properties declared by `Viewport3D`**.
Inherited properties are documented in: [SubViewportContainer](SubViewportContainer.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|
| - | id | identifier | - |
| - | model | string(url) | "" |
| - | modelSource | string(url) | "" |
| - | animation | string(url) | "" |
| - | animationSource | string(url) | "" |
| - | playAnimation | int | 0 |
| - | playFirstAnimation | bool | false |
| - | autoplayAnimation | bool | false |
| - | defaultAnimation | string | "" |
| - | playLoop | bool | false |
| - | cameraDistance | int | 0 |
| - | lightEnergy | int | 0 |

## Events

This page lists **only signals declared by `Viewport3D`**.
Inherited signals are documented in: [SubViewportContainer](SubViewportContainer.md)

| Godot Signal | SMS Event | Params |
|-|-|-|

## Actions

This page lists **only actions supported by the runtime** for `Viewport3D`.
Inherited actions are documented in: [SubViewportContainer](SubViewportContainer.md)

| Action | SMS Call | Params | Returns |
|-|-|-|-|
| playAnimation | `<id>.playAnimation(index)` | int index | void |
| stopAnimation | `<id>.stopAnimation()` | - | void |
| rewind | `<id>.rewind()` | - | void |
| setFrame | `<id>.setFrame(frame)` | int frame | void |
| zoomIn | `<id>.zoomIn()` | - | void |
| zoomOut | `<id>.zoomOut()` | - | void |
