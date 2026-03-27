# ForgeSpeechRecognizerControl

## Inheritance

[ForgeSpeechRecognizerControl](ForgeSpeechRecognizerControl.md) → [Control](Control.md) → [CanvasItem](CanvasItem.md) → [Node](Node.md) → [Object](Object.md)

## Properties

This page lists **only properties declared by `ForgeSpeechRecognizerControl`**.
Inherited properties are documented in: [Control](Control.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|
| filters | filters | string | — |
| language | language | string | — |
| mode | mode | string | — |
| suffix | suffix | string | — |

## Events

This page lists **only signals declared by `ForgeSpeechRecognizerControl`**.
Inherited signals are documented in: [Control](Control.md)

| Godot Signal | SMS Event | Params |
|-|-|-|
| error | `on <id>.error(message) { ... }` | string message |
| rawResult | `on <id>.rawResult(text) { ... }` | string text |
| result | `on <id>.result(text) { ... }` | string text |
| started | `on <id>.started() { ... }` | — |
| stopped | `on <id>.stopped() { ... }` | — |

## Runtime Actions

This page lists **callable methods declared by `ForgeSpeechRecognizerControl`**.
Inherited actions are documented in: [Control](Control.md)

| Godot Method | SMS Call | Params | Returns |
|-|-|-|-|
| listen | `<id>.listen()` | — | void |
| stop | `<id>.stop()` | — | void |
| submitText | `<id>.submitText(text)` | string text | void |

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

