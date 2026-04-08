# SubViewportContainer

## Inheritance

[SubViewportContainer](SubViewportContainer.md) → [Container](Container.md) → [Control](Control.md) → [CanvasItem](CanvasItem.md) → [Node](Node.md) → [Object](Object.md)

## Derived Classes

### Direct subclasses

- [ForgePosingEditorControl](ForgePosingEditorControl.md)
- [ForgeTimelineControl](ForgeTimelineControl.md)
- [PosingEditor](PosingEditor.md)
- [Viewport3D](Viewport3D.md)

## Properties

This page lists **only properties declared by `SubViewportContainer`**.
Inherited properties are documented in: [Container](Container.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|
| mouse_target | mouseTarget | bool | - |
| stretch | stretch | bool | - |
| stretch_shrink | stretchShrink | int | - |

## Events

This page lists **only signals declared by `SubViewportContainer`**.
Inherited signals are documented in: [Container](Container.md)

| Godot Signal | SMS Event | Params |
|-|-|-|

## Runtime Actions

This page lists **callable methods declared by `SubViewportContainer`**.
Inherited actions are documented in: [Container](Container.md)

| Godot Method | SMS Call | Params | Returns |
|-|-|-|-|
| is_mouse_target_enabled | `<id>.isMouseTargetEnabled()` | - | bool |
| is_stretch_enabled | `<id>.isStretchEnabled()` | - | bool |

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

