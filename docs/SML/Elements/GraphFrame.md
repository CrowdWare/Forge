# GraphFrame

## Inheritance

[GraphFrame](GraphFrame.md) → [GraphElement](GraphElement.md) → [Container](Container.md) → [Control](Control.md) → [CanvasItem](CanvasItem.md) → [Node](Node.md) → [Object](Object.md)

## Properties

This page lists **only properties declared by `GraphFrame`**.
Inherited properties are documented in: [GraphElement](GraphElement.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|
| autoshrink_enabled | autoshrinkEnabled | bool | - |
| autoshrink_margin | autoshrinkMargin | int | - |
| drag_margin | dragMargin | int | - |
| tint_color | tintColor | Color | - |
| tint_color_enabled | tintColorEnabled | bool | - |
| title | title | string | - |

## Events

This page lists **only signals declared by `GraphFrame`**.
Inherited signals are documented in: [GraphElement](GraphElement.md)

| Godot Signal | SMS Event | Params |
|-|-|-|
| autoshrink_changed | `on <id>.autoshrinkChanged() { ... }` | - |

## Runtime Actions

This page lists **callable methods declared by `GraphFrame`**.
Inherited actions are documented in: [GraphElement](GraphElement.md)

| Godot Method | SMS Call | Params | Returns |
|-|-|-|-|
| get_titlebar_hbox | `<id>.getTitlebarHbox()` | - | Object |

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

