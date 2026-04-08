# Label

## Inheritance

[Label](Label.md) → [Control](Control.md) → [CanvasItem](CanvasItem.md) → [Node](Node.md) → [Object](Object.md)

## Properties

This page lists **only properties declared by `Label`**.
Inherited properties are documented in: [Control](Control.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|
| font_color (theme override) | color | color | - |
| font_size (theme override) | fontSize | int | - |
| font (theme override) | font | string(path) | - |
| font (theme override, via Fonts resource block) | fontFace | string | - |
| font (theme override, via Fonts resource block) | fontWeight | identifier or int | regular |
| horizontal_alignment | horizontalAlignment | enum or int | 0 |
| normal StyleBoxFlat (theme override) | bgColor | color | - |
| normal StyleBoxFlat (theme override) | borderColor | color | - |
| normal StyleBoxFlat (theme override) | borderWidth | int | - |
| normal StyleBoxFlat (theme override) | borderRadius | int | - |

## Events

This page lists **only signals declared by `Label`**.
Inherited signals are documented in: [Control](Control.md)

| Godot Signal | SMS Event | Params |
|-|-|-|

## Runtime Actions

This page lists **callable methods declared by `Label`**.
Inherited actions are documented in: [Control](Control.md)

| Godot Method | SMS Call | Params | Returns |
|-|-|-|-|
| get_character_bounds | `<id>.getCharacterBounds(pos)` | int pos | Variant |
| get_line_count | `<id>.getLineCount()` | - | int |
| get_line_height | `<id>.getLineHeight(line)` | int line | int |
| get_total_character_count | `<id>.getTotalCharacterCount()` | - | int |
| get_visible_line_count | `<id>.getVisibleLineCount()` | - | int |
| is_clipping_text | `<id>.isClippingText()` | - | bool |

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

