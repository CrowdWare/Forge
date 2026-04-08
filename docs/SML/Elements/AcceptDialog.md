# AcceptDialog

## Inheritance

[AcceptDialog](AcceptDialog.md) → [Window](Window.md) → [Viewport](Viewport.md) → [Node](Node.md) → [Object](Object.md)

## Derived Classes

### Direct subclasses

- [ConfirmationDialog](ConfirmationDialog.md)

## Properties

This page lists **only properties declared by `AcceptDialog`**.
Inherited properties are documented in: [Window](Window.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|
| dialog_autowrap | dialogAutowrap | bool | - |
| dialog_close_on_escape | dialogCloseOnEscape | bool | - |
| dialog_hide_on_ok | dialogHideOnOk | bool | - |
| dialog_text | dialogText | string | - |
| ok_button_text | okButtonText | string | - |

## Events

This page lists **only signals declared by `AcceptDialog`**.
Inherited signals are documented in: [Window](Window.md)

| Godot Signal | SMS Event | Params |
|-|-|-|
| canceled | `on <id>.canceled() { ... }` | - |
| confirmed | `on <id>.confirmed() { ... }` | - |
| custom_action | `on <id>.customAction(action) { ... }` | Variant action |

## Runtime Actions

This page lists **callable methods declared by `AcceptDialog`**.
Inherited actions are documented in: [Window](Window.md)

| Godot Method | SMS Call | Params | Returns |
|-|-|-|-|
| add_button | `<id>.addButton(text, right, action)` | string text, bool right, string action | Object |
| add_cancel_button | `<id>.addCancelButton(name)` | string name | Object |
| get_close_on_escape | `<id>.getCloseOnEscape()` | - | bool |
| get_hide_on_ok | `<id>.getHideOnOk()` | - | bool |
| get_label | `<id>.getLabel()` | - | Object |
| get_ok_button | `<id>.getOkButton()` | - | Object |
| get_text | `<id>.getText()` | - | string |
| has_autowrap | `<id>.hasAutowrap()` | - | bool |
| set_autowrap | `<id>.setAutowrap(autowrap)` | bool autowrap | void |
| set_close_on_escape | `<id>.setCloseOnEscape(enabled)` | bool enabled | void |
| set_hide_on_ok | `<id>.setHideOnOk(enabled)` | bool enabled | void |
| set_text | `<id>.setText(text)` | string text | void |
