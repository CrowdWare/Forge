# MenuButton

## Inheritance

[MenuButton](MenuButton.md) → [Button](Button.md) → [BaseButton](BaseButton.md) → [Control](Control.md) → [CanvasItem](CanvasItem.md) → [Node](Node.md) → [Object](Object.md)

## Collection Items

This control appears to manage internal **items** (collection-style API).
Items are typically not represented as child nodes/properties in Godot.
In SML, this will be represented via **pseudo child elements** (documented per control).

## Properties

This page lists **only properties declared by `MenuButton`**.
Inherited properties are documented in: [Button](Button.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|
| item_count | itemCount | int | - |
| switch_on_hover | switchOnHover | bool | - |

## Events

This page lists **only signals declared by `MenuButton`**.
Inherited signals are documented in: [Button](Button.md)

| Godot Signal | SMS Event | Params |
|-|-|-|
| about_to_popup | `on <id>.aboutToPopup() { ... }` | - |

## Runtime Actions

This page lists **callable methods declared by `MenuButton`**.
Inherited actions are documented in: [Button](Button.md)

| Godot Method | SMS Call | Params | Returns |
|-|-|-|-|
| get_popup | `<id>.getPopup()` | - | Object |
| set_disable_shortcuts | `<id>.setDisableShortcuts(disabled)` | bool disabled | void |
| show_popup | `<id>.showPopup()` | - | void |

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


## SML Items (TODO)

This control appears to manage internal items, but a dedicated SML pseudo-child specification has not been defined yet.
Use generated runtime schema files (`ForgeRunner/Generated/Schema*.cs`) and this element reference as implementation hints.
