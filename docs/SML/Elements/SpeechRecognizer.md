# SpeechRecognizer

## Inheritance

[SpeechRecognizer](SpeechRecognizer.md) → [Control](Control.md) → [CanvasItem](CanvasItem.md) → [Node](Node.md) → [Object](Object.md)

## Properties

This page lists **only properties declared by `SpeechRecognizer`**.
Inherited properties are documented in: [Control](Control.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|
| — | id | identifier | — |
| — | language | string | de-DE |
| — | mode | enum: raw, clean, markdown | clean |
| — | filters | string | zdf,wdr,applaus,musik |
| — | suffix | string | "" |

> Current vertical-slice behavior focuses on processing pipeline and emits result/rawResult signals.
> On Android, listen() currently emits an error hint until native capture wiring is added.
> Use keyboard dictation as input source, then pass text through submitText() for Raw/Clean/Markdown processing.

### Examples

```sml
SpeechRecognizer {
    id: mic
    language: "de-DE"
    mode: clean
    filters: "zdf,wdr,applaus,musik"
}
```

## Events

This page lists **only signals declared by `SpeechRecognizer`**.
Inherited signals are documented in: [Control](Control.md)

| Godot Signal | SMS Event | Params |
|-|-|-|

## Actions

This page lists **only actions supported by the runtime** for `SpeechRecognizer`.
Inherited actions are documented in: [Control](Control.md)

| Action | SMS Call | Params | Returns |
|-|-|-|-|
| listen | `<id>.listen()` | — | void |
| stop | `<id>.stop()` | — | void |
| submitText | `<id>.submitText(text)` | string text | void |
| setMode | `<id>.setMode(value)` | string value | void |
