# LineEdit

## Inheritance

[LineEdit](LineEdit.md) → [Control](Control.md) → [CanvasItem](CanvasItem.md) → [Node](Node.md) → [Object](Object.md)

## Derived Classes

### Direct subclasses

- [ForgeNumberPickerControl](ForgeNumberPickerControl.md)

## Properties

This page lists **only properties declared by `LineEdit`**.
Inherited properties are documented in: [Control](Control.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|
| font_color (theme override) | color | color | - |
| font_size (theme override) | fontSize | int | - |
| font (theme override) | font | string(path) | - |
| font (theme override, via Fonts resource block) | fontFace | string | - |
| font (theme override, via Fonts resource block) | fontWeight | identifier or int | regular |
| normal StyleBoxFlat (theme override) | bgColor | color | - |
| normal StyleBoxFlat (theme override) | borderColor | color | - |
| normal StyleBoxFlat (theme override) | borderWidth | int | - |
| normal StyleBoxFlat (theme override) | borderRadius | int | - |

## Events

This page lists **only signals declared by `LineEdit`**.
Inherited signals are documented in: [Control](Control.md)

| Godot Signal | SMS Event | Params |
|-|-|-|
| editing_toggled | `on <id>.editingToggled(toggledOn) { ... }` | bool toggledOn |
| text_change_rejected | `on <id>.textChangeRejected(rejectedSubstring) { ... }` | string rejectedSubstring |
| text_changed | `on <id>.textChanged(newText) { ... }` | string newText |
| text_submitted | `on <id>.textSubmitted(newText) { ... }` | string newText |

## Runtime Actions

This page lists **callable methods declared by `LineEdit`**.
Inherited actions are documented in: [Control](Control.md)

| Godot Method | SMS Call | Params | Returns |
|-|-|-|-|
| apply_ime | `<id>.applyIme()` | - | void |
| cancel_ime | `<id>.cancelIme()` | - | void |
| clear | `<id>.clear()` | - | void |
| delete_char_at_caret | `<id>.deleteCharAtCaret()` | - | void |
| delete_text | `<id>.deleteText(fromColumn, toColumn)` | int fromColumn, int toColumn | void |
| deselect | `<id>.deselect()` | - | void |
| edit | `<id>.edit(hideFocus)` | bool hideFocus | void |
| get_horizontal_alignment | `<id>.getHorizontalAlignment()` | - | int |
| get_menu | `<id>.getMenu()` | - | Object |
| get_next_composite_character_column | `<id>.getNextCompositeCharacterColumn(column)` | int column | int |
| get_placeholder | `<id>.getPlaceholder()` | - | string |
| get_previous_composite_character_column | `<id>.getPreviousCompositeCharacterColumn(column)` | int column | int |
| get_scroll_offset | `<id>.getScrollOffset()` | - | float |
| get_selected_text | `<id>.getSelectedText()` | - | string |
| get_selection_from_column | `<id>.getSelectionFromColumn()` | - | int |
| get_selection_to_column | `<id>.getSelectionToColumn()` | - | int |
| has_ime_text | `<id>.hasImeText()` | - | bool |
| has_redo | `<id>.hasRedo()` | - | bool |
| has_selection | `<id>.hasSelection()` | - | bool |
| has_undo | `<id>.hasUndo()` | - | bool |
| insert_text_at_caret | `<id>.insertTextAtCaret(text)` | string text | void |
| is_caret_blink_enabled | `<id>.isCaretBlinkEnabled()` | - | bool |
| is_caret_mid_grapheme_enabled | `<id>.isCaretMidGraphemeEnabled()` | - | bool |
| is_editing | `<id>.isEditing()` | - | bool |
| is_editing_kept_on_text_submit | `<id>.isEditingKeptOnTextSubmit()` | - | bool |
| is_expand_to_text_length_enabled | `<id>.isExpandToTextLengthEnabled()` | - | bool |
| is_menu_visible | `<id>.isMenuVisible()` | - | bool |
| menu_option | `<id>.menuOption(option)` | int option | void |
| select | `<id>.select(from, to)` | int from, int to | void |
| select_all | `<id>.selectAll()` | - | void |
| set_caret_blink_enabled | `<id>.setCaretBlinkEnabled(enabled)` | bool enabled | void |
| set_caret_mid_grapheme_enabled | `<id>.setCaretMidGraphemeEnabled(enabled)` | bool enabled | void |
| set_expand_to_text_length_enabled | `<id>.setExpandToTextLengthEnabled(enabled)` | bool enabled | void |
| set_horizontal_alignment | `<id>.setHorizontalAlignment(alignment)` | int alignment | void |
| set_placeholder | `<id>.setPlaceholder(text)` | string text | void |
| unedit | `<id>.unedit()` | - | void |

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

