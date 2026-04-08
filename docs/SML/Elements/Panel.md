# Panel

## Inheritance

[Panel](Panel.md) → [Control](Control.md) → [CanvasItem](CanvasItem.md) → [Node](Node.md) → [Object](Object.md)

## Derived Classes

### Direct subclasses

- [SplashScreen](SplashScreen.md)
- [Terminal](Terminal.md)
- [WindowDrag](WindowDrag.md)

## Properties

This page lists **only properties declared by `Panel`**.
Inherited properties are documented in: [Control](Control.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|
| panel StyleBoxFlat (theme override) | bgColor | color | - |
| panel StyleBoxFlat (theme override) | borderColor | color | - |
| panel StyleBoxFlat (theme override) | borderWidth | int | - |
| panel StyleBoxFlat (theme override) | borderRadius | int | - |
| StyleBoxFlat.shadow_color | shadowColor | color | - |
| StyleBoxFlat.shadow_size | shadowSize | int | - |
| StyleBoxFlat.shadow_offset.x | shadowOffsetX | int | 0 |
| StyleBoxFlat.shadow_offset.y | shadowOffsetY | int | 0 |
| - (expands to profile properties before build) | elevation | identifier | - |

## Events

This page lists **only signals declared by `Panel`**.
Inherited signals are documented in: [Control](Control.md)

| Godot Signal | SMS Event | Params |
|-|-|-|

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

