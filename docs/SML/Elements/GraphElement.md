# GraphElement

## Inheritance

[GraphElement](GraphElement.md) → [Container](Container.md) → [Control](Control.md) → [CanvasItem](CanvasItem.md) → [Node](Node.md) → [Object](Object.md)

## Derived Classes

### Direct subclasses

- [GraphFrame](GraphFrame.md)
- [GraphNode](GraphNode.md)

## Properties

This page lists **only properties declared by `GraphElement`**.
Inherited properties are documented in: [Container](Container.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|
| draggable | draggable | bool | - |
| position_offset | positionOffset | Vector2 | - |
| resizable | resizable | bool | - |
| scaling_menus | scalingMenus | bool | - |
| selectable | selectable | bool | - |
| selected | selected | bool | - |

## Events

This page lists **only signals declared by `GraphElement`**.
Inherited signals are documented in: [Container](Container.md)

| Godot Signal | SMS Event | Params |
|-|-|-|
| delete_request | `on <id>.deleteRequest() { ... }` | - |
| dragged | `on <id>.dragged(from, to) { ... }` | Vector2 from, Vector2 to |
| node_deselected | `on <id>.nodeDeselected() { ... }` | - |
| node_selected | `on <id>.nodeSelected() { ... }` | - |
| position_offset_changed | `on <id>.positionOffsetChanged() { ... }` | - |
| raise_request | `on <id>.raiseRequest() { ... }` | - |
| resize_end | `on <id>.resizeEnd(newSize) { ... }` | Vector2 newSize |
| resize_request | `on <id>.resizeRequest(newSize) { ... }` | Vector2 newSize |

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

