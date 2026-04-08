# MenuBar

## Inheritance

[MenuBar](MenuBar.md) → [Control](Control.md) → [CanvasItem](CanvasItem.md) → [Node](Node.md) → [Object](Object.md)

## Properties

This page lists **only properties declared by `MenuBar`**.
Inherited properties are documented in: [Control](Control.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|
| flat | flat | bool | - |
| language | language | string | - |
| prefer_global_menu | preferGlobalMenu | bool | - |
| start_index | startIndex | int | - |
| switch_on_hover | switchOnHover | bool | - |
| text_direction | textDirection | int | - |

## Events

This page lists **only signals declared by `MenuBar`**.
Inherited signals are documented in: [Control](Control.md)

| Godot Signal | SMS Event | Params |
|-|-|-|

## Runtime Actions

This page lists **callable methods declared by `MenuBar`**.
Inherited actions are documented in: [Control](Control.md)

| Godot Method | SMS Call | Params | Returns |
|-|-|-|-|
| get_menu_count | `<id>.getMenuCount()` | - | int |
| get_menu_popup | `<id>.getMenuPopup(menu)` | int menu | Object |
| get_menu_title | `<id>.getMenuTitle(menu)` | int menu | string |
| get_menu_tooltip | `<id>.getMenuTooltip(menu)` | int menu | string |
| is_menu_disabled | `<id>.isMenuDisabled(menu)` | int menu | bool |
| is_menu_hidden | `<id>.isMenuHidden(menu)` | int menu | bool |
| is_native_menu | `<id>.isNativeMenu()` | - | bool |
| set_disable_shortcuts | `<id>.setDisableShortcuts(disabled)` | bool disabled | void |
| set_menu_disabled | `<id>.setMenuDisabled(menu, disabled)` | int menu, bool disabled | void |
| set_menu_hidden | `<id>.setMenuHidden(menu, hidden)` | int menu, bool hidden | void |
| set_menu_title | `<id>.setMenuTitle(menu, title)` | int menu, string title | void |
| set_menu_tooltip | `<id>.setMenuTooltip(menu, tooltip)` | int menu, string tooltip | void |

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

