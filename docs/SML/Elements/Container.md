# Container

## Inheritance

[Container](Container.md) → [Control](Control.md) → [CanvasItem](CanvasItem.md) → [Node](Node.md) → [Object](Object.md)

## Derived Classes

### Direct subclasses

- [AspectRatioContainer](AspectRatioContainer.md)
- [BoxContainer](BoxContainer.md)
- [CenterContainer](CenterContainer.md)
- [DockingHost](DockingHost.md)
- [EditorProperty](EditorProperty.md)
- [FlowContainer](FlowContainer.md)
- [FoldableContainer](FoldableContainer.md)
- [ForgeDockingHostControl](ForgeDockingHostControl.md)
- [GraphElement](GraphElement.md)
- [GridContainer](GridContainer.md)
- [MarginContainer](MarginContainer.md)
- [PanelContainer](PanelContainer.md)
- [ScrollContainer](ScrollContainer.md)
- [SplitContainer](SplitContainer.md)
- [SubViewportContainer](SubViewportContainer.md)
- [TabContainer](TabContainer.md)

## Properties

This page lists **only properties declared by `Container`**.
Inherited properties are documented in: [Control](Control.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|

## Events

This page lists **only signals declared by `Container`**.
Inherited signals are documented in: [Control](Control.md)

| Godot Signal | SMS Event | Params |
|-|-|-|
| pre_sort_children | `on <id>.preSortChildren() { ... }` | - |
| sort_children | `on <id>.sortChildren() { ... }` | - |

## Runtime Actions

This page lists **callable methods declared by `Container`**.
Inherited actions are documented in: [Control](Control.md)

| Godot Method | SMS Call | Params | Returns |
|-|-|-|-|
| queue_sort | `<id>.queueSort()` | - | void |

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

