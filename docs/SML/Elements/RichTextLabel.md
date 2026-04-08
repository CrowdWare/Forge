# RichTextLabel

## Inheritance

[RichTextLabel](RichTextLabel.md) → [Control](Control.md) → [CanvasItem](CanvasItem.md) → [Node](Node.md) → [Object](Object.md)

## Properties

This page lists **only properties declared by `RichTextLabel`**.
Inherited properties are documented in: [Control](Control.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|
| default_color (theme override) | color | color | - |
| normal_font_size (theme override) | fontSize | int | - |
| normal_font (theme override) | font | string(path) | - |
| normal_font (theme override, via Fonts resource block) | fontFace | string | - |
| normal_font (theme override, via Fonts resource block) | fontWeight | identifier or int | regular |
| horizontal_alignment | horizontalAlignment | enum or int | 0 |

## Events

This page lists **only signals declared by `RichTextLabel`**.
Inherited signals are documented in: [Control](Control.md)

| Godot Signal | SMS Event | Params |
|-|-|-|
| finished | `on <id>.finished() { ... }` | - |
| meta_clicked | `on <id>.metaClicked(meta) { ... }` | Variant meta |
| meta_hover_ended | `on <id>.metaHoverEnded(meta) { ... }` | Variant meta |
| meta_hover_started | `on <id>.metaHoverStarted(meta) { ... }` | Variant meta |

## Runtime Actions

This page lists **callable methods declared by `RichTextLabel`**.
Inherited actions are documented in: [Control](Control.md)

| Godot Method | SMS Call | Params | Returns |
|-|-|-|-|
| add_hr | `<id>.addHr(width, height, color, alignment, widthInPercent, heightInPercent)` | int width, int height, Color color, int alignment, bool widthInPercent, bool heightInPercent | void |
| add_text | `<id>.addText(text)` | string text | void |
| append_text | `<id>.appendText(bbcode)` | string bbcode | void |
| clear | `<id>.clear()` | - | void |
| deselect | `<id>.deselect()` | - | void |
| get_character_line | `<id>.getCharacterLine(character)` | int character | int |
| get_character_paragraph | `<id>.getCharacterParagraph(character)` | int character | int |
| get_content_height | `<id>.getContentHeight()` | - | int |
| get_content_width | `<id>.getContentWidth()` | - | int |
| get_effects | `<id>.getEffects()` | - | Variant |
| get_line_count | `<id>.getLineCount()` | - | int |
| get_line_height | `<id>.getLineHeight(line)` | int line | int |
| get_line_offset | `<id>.getLineOffset(line)` | int line | float |
| get_line_range | `<id>.getLineRange(line)` | int line | Variant |
| get_line_width | `<id>.getLineWidth(line)` | int line | int |
| get_menu | `<id>.getMenu()` | - | Object |
| get_paragraph_count | `<id>.getParagraphCount()` | - | int |
| get_paragraph_offset | `<id>.getParagraphOffset(paragraph)` | int paragraph | float |
| get_parsed_text | `<id>.getParsedText()` | - | string |
| get_selected_text | `<id>.getSelectedText()` | - | string |
| get_selection_from | `<id>.getSelectionFrom()` | - | int |
| get_selection_line_offset | `<id>.getSelectionLineOffset()` | - | float |
| get_selection_to | `<id>.getSelectionTo()` | - | int |
| get_total_character_count | `<id>.getTotalCharacterCount()` | - | int |
| get_v_scroll_bar | `<id>.getVScrollBar()` | - | Object |
| get_visible_content_rect | `<id>.getVisibleContentRect()` | - | Variant |
| get_visible_line_count | `<id>.getVisibleLineCount()` | - | int |
| get_visible_paragraph_count | `<id>.getVisibleParagraphCount()` | - | int |
| invalidate_paragraph | `<id>.invalidateParagraph(paragraph)` | int paragraph | bool |
| is_finished | `<id>.isFinished()` | - | bool |
| is_fit_content_enabled | `<id>.isFitContentEnabled()` | - | bool |
| is_menu_visible | `<id>.isMenuVisible()` | - | bool |
| is_ready | `<id>.isReady()` | - | bool |
| is_using_bbcode | `<id>.isUsingBbcode()` | - | bool |
| menu_option | `<id>.menuOption(option)` | int option | void |
| newline | `<id>.newline()` | - | void |
| parse_bbcode | `<id>.parseBbcode(bbcode)` | string bbcode | void |
| pop | `<id>.pop()` | - | void |
| pop_all | `<id>.popAll()` | - | void |
| pop_context | `<id>.popContext()` | - | void |
| push_bgcolor | `<id>.pushBgcolor(bgcolor)` | Color bgcolor | void |
| push_bold | `<id>.pushBold()` | - | void |
| push_bold_italics | `<id>.pushBoldItalics()` | - | void |
| push_cell | `<id>.pushCell()` | - | void |
| push_color | `<id>.pushColor(color)` | Color color | void |
| push_context | `<id>.pushContext()` | - | void |
| push_fgcolor | `<id>.pushFgcolor(fgcolor)` | Color fgcolor | void |
| push_font_size | `<id>.pushFontSize(fontSize)` | int fontSize | void |
| push_hint | `<id>.pushHint(description)` | string description | void |
| push_indent | `<id>.pushIndent(level)` | int level | void |
| push_italics | `<id>.pushItalics()` | - | void |
| push_language | `<id>.pushLanguage(language)` | string language | void |
| push_list | `<id>.pushList(level, type, capitalize, bullet)` | int level, int type, bool capitalize, string bullet | void |
| push_mono | `<id>.pushMono()` | - | void |
| push_normal | `<id>.pushNormal()` | - | void |
| push_outline_color | `<id>.pushOutlineColor(color)` | Color color | void |
| push_outline_size | `<id>.pushOutlineSize(outlineSize)` | int outlineSize | void |
| push_strikethrough | `<id>.pushStrikethrough(color)` | Color color | void |
| push_table | `<id>.pushTable(columns, inlineAlign, alignToRow, name)` | int columns, int inlineAlign, int alignToRow, string name | void |
| push_underline | `<id>.pushUnderline(color)` | Color color | void |
| reload_effects | `<id>.reloadEffects()` | - | void |
| remove_paragraph | `<id>.removeParagraph(paragraph, noInvalidate)` | int paragraph, bool noInvalidate | bool |
| scroll_to_line | `<id>.scrollToLine(line)` | int line | void |
| scroll_to_paragraph | `<id>.scrollToParagraph(paragraph)` | int paragraph | void |
| scroll_to_selection | `<id>.scrollToSelection()` | - | void |
| select_all | `<id>.selectAll()` | - | void |
| set_cell_border_color | `<id>.setCellBorderColor(color)` | Color color | void |
| set_cell_row_background_color | `<id>.setCellRowBackgroundColor(oddRowBg, evenRowBg)` | Color oddRowBg, Color evenRowBg | void |
| set_cell_size_override | `<id>.setCellSizeOverride(minSize, maxSize)` | Vector2 minSize, Vector2 maxSize | void |
| set_hint_underline | `<id>.setHintUnderline(enable)` | bool enable | void |
| set_meta_underline | `<id>.setMetaUnderline(enable)` | bool enable | void |
| set_scroll_follow | `<id>.setScrollFollow(follow)` | bool follow | void |
| set_scroll_follow_visible_characters | `<id>.setScrollFollowVisibleCharacters(follow)` | bool follow | void |
| set_table_column_expand | `<id>.setTableColumnExpand(column, expand, ratio, shrink)` | int column, bool expand, int ratio, bool shrink | void |
| set_table_column_name | `<id>.setTableColumnName(column, name)` | int column, string name | void |
| set_use_bbcode | `<id>.setUseBbcode(enable)` | bool enable | void |

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

