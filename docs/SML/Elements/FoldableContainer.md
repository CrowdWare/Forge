# FoldableContainer

## Inheritance

[FoldableContainer](FoldableContainer.md) → [Container](Container.md) → [Control](Control.md) → [CanvasItem](CanvasItem.md) → [Node](Node.md) → [Object](Object.md)

## Properties

This page lists **only properties declared by `FoldableContainer`**.
Inherited properties are documented in: [Container](Container.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|
| folded | folded | bool | - |
| language | language | string | - |
| title | title | string | - |
| title_alignment | titleAlignment | int | - |
| title_position | titlePosition | int | - |
| title_text_direction | titleTextDirection | int | - |
| title_text_overrun_behavior | titleTextOverrunBehavior | int | - |

## Events

This page lists **only signals declared by `FoldableContainer`**.
Inherited signals are documented in: [Container](Container.md)

| Godot Signal | SMS Event | Params |
|-|-|-|
| folding_changed | `on <id>.foldingChanged(isFolded) { ... }` | bool isFolded |

## Runtime Actions

This page lists **callable methods declared by `FoldableContainer`**.
Inherited actions are documented in: [Container](Container.md)

| Godot Method | SMS Call | Params | Returns |
|-|-|-|-|
| expand | `<id>.expand()` | - | void |
| fold | `<id>.fold()` | - | void |

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

