# ForgeDockingContainerControl

## Inheritance

[ForgeDockingContainerControl](ForgeDockingContainerControl.md) → [TabContainer](TabContainer.md) → [Container](Container.md) → [Control](Control.md) → [CanvasItem](CanvasItem.md) → [Node](Node.md) → [Object](Object.md)

## Properties

This page lists **only properties declared by `ForgeDockingContainerControl`**.
Inherited properties are documented in: [TabContainer](TabContainer.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|

## Events

This page lists **only signals declared by `ForgeDockingContainerControl`**.
Inherited signals are documented in: [TabContainer](TabContainer.md)

| Godot Signal | SMS Event | Params |
|-|-|-|

## Runtime Actions

This page lists **callable methods declared by `ForgeDockingContainerControl`**.
Inherited actions are documented in: [TabContainer](TabContainer.md)

| Godot Method | SMS Call | Params | Returns |
|-|-|-|-|
| get_base_fixed_width | `<id>.getBaseFixedWidth()` | - | float |
| get_dock_side | `<id>.getDockSide()` | - | string |
| get_fixed_height | `<id>.getFixedHeight()` | - | float |
| get_fixed_width | `<id>.getFixedWidth()` | - | float |
| get_height_percent | `<id>.getHeightPercent()` | - | float |
| is_collapsed | `<id>.isCollapsed()` | - | bool |
| is_flex | `<id>.isFlex()` | - | bool |
| set_collapsed | `<id>.setCollapsed(value)` | bool value | void |
| set_dock_side | `<id>.setDockSide(side)` | string side | void |
| set_fixed_height | `<id>.setFixedHeight(value)` | float value | void |
| set_fixed_width | `<id>.setFixedWidth(value)` | float value | void |
| set_flex | `<id>.setFlex(value)` | bool value | void |
| set_height_percent | `<id>.setHeightPercent(value)` | float value | void |

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

