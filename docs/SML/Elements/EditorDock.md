# EditorDock

## Inheritance

[EditorDock](EditorDock.md) → [MarginContainer](MarginContainer.md) → [Container](Container.md) → [Control](Control.md) → [CanvasItem](CanvasItem.md) → [Node](Node.md) → [Object](Object.md)

## Derived Classes

### Direct subclasses

- [FileSystemDock](FileSystemDock.md)

## Properties

This page lists **only properties declared by `EditorDock`**.
Inherited properties are documented in: [MarginContainer](MarginContainer.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|
| available_layouts | availableLayouts | int | - |
| closable | closable | bool | - |
| default_slot | defaultSlot | int | - |
| force_show_icon | forceShowIcon | bool | - |
| global | global | bool | - |
| layout_key | layoutKey | string | - |
| title | title | string | - |
| title_color | titleColor | Color | - |
| transient | transient | bool | - |

## Events

This page lists **only signals declared by `EditorDock`**.
Inherited signals are documented in: [MarginContainer](MarginContainer.md)

| Godot Signal | SMS Event | Params |
|-|-|-|
| _tab_style_changed | `on <id>.TabStyleChanged() { ... }` | - |
| closed | `on <id>.closed() { ... }` | - |

## Runtime Actions

This page lists **callable methods declared by `EditorDock`**.
Inherited actions are documented in: [MarginContainer](MarginContainer.md)

| Godot Method | SMS Call | Params | Returns |
|-|-|-|-|
| close | `<id>.close()` | - | void |
| make_visible | `<id>.makeVisible()` | - | void |
| open | `<id>.open()` | - | void |

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

