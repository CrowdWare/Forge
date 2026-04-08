# LinkButton

## Inheritance

[LinkButton](LinkButton.md) → [BaseButton](BaseButton.md) → [Control](Control.md) → [CanvasItem](CanvasItem.md) → [Node](Node.md) → [Object](Object.md)

## Properties

This page lists **only properties declared by `LinkButton`**.
Inherited properties are documented in: [BaseButton](BaseButton.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|
| ellipsis_char | ellipsisChar | string | - |
| language | language | string | - |
| structured_text_bidi_override | structuredTextBidiOverride | int | - |
| text | text | string | - |
| text_direction | textDirection | int | - |
| text_overrun_behavior | textOverrunBehavior | int | - |
| underline | underline | int | - |
| uri | uri | string | - |

## Events

This page lists **only signals declared by `LinkButton`**.
Inherited signals are documented in: [BaseButton](BaseButton.md)

| Godot Signal | SMS Event | Params |
|-|-|-|

## Runtime Actions

This page lists **callable methods declared by `LinkButton`**.
Inherited actions are documented in: [BaseButton](BaseButton.md)

| Godot Method | SMS Call | Params | Returns |
|-|-|-|-|
| get_underline_mode | `<id>.getUnderlineMode()` | - | int |
| set_underline_mode | `<id>.setUnderlineMode(underlineMode)` | int underlineMode | void |

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

