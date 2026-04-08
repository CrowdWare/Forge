# TextEdit

## Inheritance

[TextEdit](TextEdit.md) → [Control](Control.md) → [CanvasItem](CanvasItem.md) → [Node](Node.md) → [Object](Object.md)

## Derived Classes

### Direct subclasses

- [CodeEdit](CodeEdit.md)

## Properties

This page lists **only properties declared by `TextEdit`**.
Inherited properties are documented in: [Control](Control.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|
| font_color (theme override) | color | color | - |
| font_size (theme override) | fontSize | int | - |
| font (theme override) | font | string(path) | - |
| font (theme override, via Fonts resource block) | fontFace | string | - |
| font (theme override, via Fonts resource block) | fontWeight | identifier or int | regular |
| normal/read_only/focus StyleBoxFlat (theme override) | bgColor | color | - |
| normal StyleBoxFlat (theme override) | borderColor | color | - |
| normal StyleBoxFlat (theme override) | borderWidth | int | - |
| normal StyleBoxFlat (theme override) | borderRadius | int | - |

## Events

This page lists **only signals declared by `TextEdit`**.
Inherited signals are documented in: [Control](Control.md)

| Godot Signal | SMS Event | Params |
|-|-|-|
| caret_changed | `on <id>.caretChanged() { ... }` | - |
| gutter_added | `on <id>.gutterAdded() { ... }` | - |
| gutter_clicked | `on <id>.gutterClicked(line, gutter) { ... }` | int line, int gutter |
| gutter_removed | `on <id>.gutterRemoved() { ... }` | - |
| lines_edited_from | `on <id>.linesEditedFrom(fromLine, toLine) { ... }` | int fromLine, int toLine |
| text_changed | `on <id>.textChanged() { ... }` | - |
| text_set | `on <id>.textSet() { ... }` | - |

## Runtime Actions

This page lists **callable methods declared by `TextEdit`**.
Inherited actions are documented in: [Control](Control.md)

| Godot Method | SMS Call | Params | Returns |
|-|-|-|-|
| add_caret | `<id>.addCaret(line, column)` | int line, int column | int |
| add_caret_at_carets | `<id>.addCaretAtCarets(below)` | bool below | void |
| add_gutter | `<id>.addGutter(at)` | int at | void |
| add_selection_for_next_occurrence | `<id>.addSelectionForNextOccurrence()` | - | void |
| adjust_carets_after_edit | `<id>.adjustCaretsAfterEdit(caret, fromLine, fromCol, toLine, toCol)` | int caret, int fromLine, int fromCol, int toLine, int toCol | void |
| adjust_viewport_to_caret | `<id>.adjustViewportToCaret(caretIndex)` | int caretIndex | void |
| apply_ime | `<id>.applyIme()` | - | void |
| backspace | `<id>.backspace(caretIndex)` | int caretIndex | void |
| begin_complex_operation | `<id>.beginComplexOperation()` | - | void |
| begin_multicaret_edit | `<id>.beginMulticaretEdit()` | - | void |
| cancel_ime | `<id>.cancelIme()` | - | void |
| center_viewport_to_caret | `<id>.centerViewportToCaret(caretIndex)` | int caretIndex | void |
| clear | `<id>.clear()` | - | void |
| clear_undo_history | `<id>.clearUndoHistory()` | - | void |
| collapse_carets | `<id>.collapseCarets(fromLine, fromColumn, toLine, toColumn, inclusive)` | int fromLine, int fromColumn, int toLine, int toColumn, bool inclusive | void |
| copy | `<id>.copy(caretIndex)` | int caretIndex | void |
| cut | `<id>.cut(caretIndex)` | int caretIndex | void |
| delete_selection | `<id>.deleteSelection(caretIndex)` | int caretIndex | void |
| deselect | `<id>.deselect(caretIndex)` | int caretIndex | void |
| end_action | `<id>.endAction()` | - | void |
| end_complex_operation | `<id>.endComplexOperation()` | - | void |
| end_multicaret_edit | `<id>.endMulticaretEdit()` | - | void |
| get_caret_column | `<id>.getCaretColumn(caretIndex)` | int caretIndex | int |
| get_caret_count | `<id>.getCaretCount()` | - | int |
| get_caret_draw_pos | `<id>.getCaretDrawPos(caretIndex)` | int caretIndex | Vector2 |
| get_caret_index_edit_order | `<id>.getCaretIndexEditOrder()` | - | Variant |
| get_caret_line | `<id>.getCaretLine(caretIndex)` | int caretIndex | int |
| get_caret_wrap_index | `<id>.getCaretWrapIndex(caretIndex)` | int caretIndex | int |
| get_first_non_whitespace_column | `<id>.getFirstNonWhitespaceColumn(line)` | int line | int |
| get_first_visible_line | `<id>.getFirstVisibleLine()` | - | int |
| get_gutter_count | `<id>.getGutterCount()` | - | int |
| get_gutter_name | `<id>.getGutterName(gutter)` | int gutter | string |
| get_gutter_type | `<id>.getGutterType(gutter)` | int gutter | int |
| get_gutter_width | `<id>.getGutterWidth(gutter)` | int gutter | int |
| get_h_scroll | `<id>.getHScroll()` | - | int |
| get_h_scroll_bar | `<id>.getHScrollBar()` | - | Object |
| get_indent_level | `<id>.getIndentLevel(line)` | int line | int |
| get_last_full_visible_line | `<id>.getLastFullVisibleLine()` | - | int |
| get_last_full_visible_line_wrap_index | `<id>.getLastFullVisibleLineWrapIndex()` | - | int |
| get_last_unhidden_line | `<id>.getLastUnhiddenLine()` | - | int |
| get_line | `<id>.getLine(line)` | int line | string |
| get_line_background_color | `<id>.getLineBackgroundColor(line)` | int line | Color |
| get_line_count | `<id>.getLineCount()` | - | int |
| get_line_gutter_icon | `<id>.getLineGutterIcon(line, gutter)` | int line, int gutter | Object |
| get_line_gutter_item_color | `<id>.getLineGutterItemColor(line, gutter)` | int line, int gutter | Color |
| get_line_gutter_metadata | `<id>.getLineGutterMetadata(line, gutter)` | int line, int gutter | void |
| get_line_gutter_text | `<id>.getLineGutterText(line, gutter)` | int line, int gutter | string |
| get_line_height | `<id>.getLineHeight()` | - | int |
| get_line_ranges_from_carets | `<id>.getLineRangesFromCarets(onlySelections, mergeAdjacent)` | bool onlySelections, bool mergeAdjacent | Variant |
| get_line_width | `<id>.getLineWidth(line, wrapIndex)` | int line, int wrapIndex | int |
| get_line_with_ime | `<id>.getLineWithIme(line)` | int line | string |
| get_line_wrap_count | `<id>.getLineWrapCount(line)` | int line | int |
| get_line_wrap_index_at_column | `<id>.getLineWrapIndexAtColumn(line, column)` | int line, int column | int |
| get_line_wrapped_text | `<id>.getLineWrappedText(line)` | int line | Variant |
| get_line_wrapping_mode | `<id>.getLineWrappingMode()` | - | int |
| get_local_mouse_pos | `<id>.getLocalMousePos()` | - | Vector2 |
| get_menu | `<id>.getMenu()` | - | Object |
| get_minimap_visible_lines | `<id>.getMinimapVisibleLines()` | - | int |
| get_next_composite_character_column | `<id>.getNextCompositeCharacterColumn(line, column)` | int line, int column | int |
| get_next_visible_line_index_offset_from | `<id>.getNextVisibleLineIndexOffsetFrom(line, wrapIndex, visibleAmount)` | int line, int wrapIndex, int visibleAmount | Variant |
| get_next_visible_line_offset_from | `<id>.getNextVisibleLineOffsetFrom(line, visibleAmount)` | int line, int visibleAmount | int |
| get_placeholder | `<id>.getPlaceholder()` | - | string |
| get_pos_at_line_column | `<id>.getPosAtLineColumn(line, column)` | int line, int column | Variant |
| get_previous_composite_character_column | `<id>.getPreviousCompositeCharacterColumn(line, column)` | int line, int column | int |
| get_rect_at_line_column | `<id>.getRectAtLineColumn(line, column)` | int line, int column | Variant |
| get_saved_version | `<id>.getSavedVersion()` | - | int |
| get_scroll_pos_for_line | `<id>.getScrollPosForLine(line, wrapIndex)` | int line, int wrapIndex | float |
| get_selected_text | `<id>.getSelectedText(caretIndex)` | int caretIndex | string |
| get_selection_at_line_column | `<id>.getSelectionAtLineColumn(line, column, includeEdges, onlySelections)` | int line, int column, bool includeEdges, bool onlySelections | int |
| get_selection_column | `<id>.getSelectionColumn(caretIndex)` | int caretIndex | int |
| get_selection_from_column | `<id>.getSelectionFromColumn(caretIndex)` | int caretIndex | int |
| get_selection_from_line | `<id>.getSelectionFromLine(caretIndex)` | int caretIndex | int |
| get_selection_line | `<id>.getSelectionLine(caretIndex)` | int caretIndex | int |
| get_selection_mode | `<id>.getSelectionMode()` | - | int |
| get_selection_origin_column | `<id>.getSelectionOriginColumn(caretIndex)` | int caretIndex | int |
| get_selection_origin_line | `<id>.getSelectionOriginLine(caretIndex)` | int caretIndex | int |
| get_selection_to_column | `<id>.getSelectionToColumn(caretIndex)` | int caretIndex | int |
| get_selection_to_line | `<id>.getSelectionToLine(caretIndex)` | int caretIndex | int |
| get_sorted_carets | `<id>.getSortedCarets(includeIgnoredCarets)` | bool includeIgnoredCarets | Variant |
| get_tab_size | `<id>.getTabSize()` | - | int |
| get_total_gutter_width | `<id>.getTotalGutterWidth()` | - | int |
| get_total_visible_line_count | `<id>.getTotalVisibleLineCount()` | - | int |
| get_v_scroll | `<id>.getVScroll()` | - | float |
| get_v_scroll_bar | `<id>.getVScrollBar()` | - | Object |
| get_v_scroll_speed | `<id>.getVScrollSpeed()` | - | float |
| get_version | `<id>.getVersion()` | - | int |
| get_visible_line_count | `<id>.getVisibleLineCount()` | - | int |
| get_visible_line_count_in_range | `<id>.getVisibleLineCountInRange(fromLine, toLine)` | int fromLine, int toLine | int |
| get_word_at_pos | `<id>.getWordAtPos(position)` | Vector2 position | string |
| get_word_under_caret | `<id>.getWordUnderCaret(caretIndex)` | int caretIndex | string |
| has_ime_text | `<id>.hasImeText()` | - | bool |
| has_redo | `<id>.hasRedo()` | - | bool |
| has_selection | `<id>.hasSelection(caretIndex)` | int caretIndex | bool |
| has_undo | `<id>.hasUndo()` | - | bool |
| insert_line_at | `<id>.insertLineAt(line, text)` | int line, string text | void |
| insert_text | `<id>.insertText(text, line, column, beforeSelectionBegin, beforeSelectionEnd)` | string text, int line, int column, bool beforeSelectionBegin, bool beforeSelectionEnd | void |
| insert_text_at_caret | `<id>.insertTextAtCaret(text, caretIndex)` | string text, int caretIndex | void |
| is_caret_after_selection_origin | `<id>.isCaretAfterSelectionOrigin(caretIndex)` | int caretIndex | bool |
| is_caret_blink_enabled | `<id>.isCaretBlinkEnabled()` | - | bool |
| is_caret_mid_grapheme_enabled | `<id>.isCaretMidGraphemeEnabled()` | - | bool |
| is_caret_visible | `<id>.isCaretVisible(caretIndex)` | int caretIndex | bool |
| is_custom_word_separators_enabled | `<id>.isCustomWordSeparatorsEnabled()` | - | bool |
| is_default_word_separators_enabled | `<id>.isDefaultWordSeparatorsEnabled()` | - | bool |
| is_dragging_cursor | `<id>.isDraggingCursor()` | - | bool |
| is_drawing_caret_when_editable_disabled | `<id>.isDrawingCaretWhenEditableDisabled()` | - | bool |
| is_drawing_minimap | `<id>.isDrawingMinimap()` | - | bool |
| is_drawing_spaces | `<id>.isDrawingSpaces()` | - | bool |
| is_drawing_tabs | `<id>.isDrawingTabs()` | - | bool |
| is_fit_content_height_enabled | `<id>.isFitContentHeightEnabled()` | - | bool |
| is_fit_content_width_enabled | `<id>.isFitContentWidthEnabled()` | - | bool |
| is_gutter_clickable | `<id>.isGutterClickable(gutter)` | int gutter | bool |
| is_gutter_drawn | `<id>.isGutterDrawn(gutter)` | int gutter | bool |
| is_gutter_overwritable | `<id>.isGutterOverwritable(gutter)` | int gutter | bool |
| is_highlight_all_occurrences_enabled | `<id>.isHighlightAllOccurrencesEnabled()` | - | bool |
| is_highlight_current_line_enabled | `<id>.isHighlightCurrentLineEnabled()` | - | bool |
| is_in_mulitcaret_edit | `<id>.isInMulitcaretEdit()` | - | bool |
| is_line_gutter_clickable | `<id>.isLineGutterClickable(line, gutter)` | int line, int gutter | bool |
| is_line_wrapped | `<id>.isLineWrapped(line)` | int line | bool |
| is_menu_visible | `<id>.isMenuVisible()` | - | bool |
| is_mouse_over_selection | `<id>.isMouseOverSelection(edges, caretIndex)` | bool edges, int caretIndex | bool |
| is_move_caret_on_right_click_enabled | `<id>.isMoveCaretOnRightClickEnabled()` | - | bool |
| is_multiple_carets_enabled | `<id>.isMultipleCaretsEnabled()` | - | bool |
| is_overtype_mode_enabled | `<id>.isOvertypeModeEnabled()` | - | bool |
| is_scroll_past_end_of_file_enabled | `<id>.isScrollPastEndOfFileEnabled()` | - | bool |
| is_smooth_scroll_enabled | `<id>.isSmoothScrollEnabled()` | - | bool |
| menu_option | `<id>.menuOption(option)` | int option | void |
| merge_gutters | `<id>.mergeGutters(fromLine, toLine)` | int fromLine, int toLine | void |
| merge_overlapping_carets | `<id>.mergeOverlappingCarets()` | - | void |
| multicaret_edit_ignore_caret | `<id>.multicaretEditIgnoreCaret(caretIndex)` | int caretIndex | bool |
| paste | `<id>.paste(caretIndex)` | int caretIndex | void |
| paste_primary_clipboard | `<id>.pastePrimaryClipboard(caretIndex)` | int caretIndex | void |
| redo | `<id>.redo()` | - | void |
| remove_caret | `<id>.removeCaret(caret)` | int caret | void |
| remove_gutter | `<id>.removeGutter(gutter)` | int gutter | void |
| remove_line_at | `<id>.removeLineAt(line, moveCaretsDown)` | int line, bool moveCaretsDown | void |
| remove_secondary_carets | `<id>.removeSecondaryCarets()` | - | void |
| remove_text | `<id>.removeText(fromLine, fromColumn, toLine, toColumn)` | int fromLine, int fromColumn, int toLine, int toColumn | void |
| search | `<id>.search(text, flags, fromLine, fromColumn)` | string text, int flags, int fromLine, int fromColumn | Variant |
| select | `<id>.select(originLine, originColumn, caretLine, caretColumn, caretIndex)` | int originLine, int originColumn, int caretLine, int caretColumn, int caretIndex | void |
| select_all | `<id>.selectAll()` | - | void |
| select_word_under_caret | `<id>.selectWordUnderCaret(caretIndex)` | int caretIndex | void |
| set_caret_blink_enabled | `<id>.setCaretBlinkEnabled(enable)` | bool enable | void |
| set_caret_column | `<id>.setCaretColumn(column, adjustViewport, caretIndex)` | int column, bool adjustViewport, int caretIndex | void |
| set_caret_line | `<id>.setCaretLine(line, adjustViewport, canBeHidden, wrapIndex, caretIndex)` | int line, bool adjustViewport, bool canBeHidden, int wrapIndex, int caretIndex | void |
| set_caret_mid_grapheme_enabled | `<id>.setCaretMidGraphemeEnabled(enabled)` | bool enabled | void |
| set_draw_caret_when_editable_disabled | `<id>.setDrawCaretWhenEditableDisabled(enable)` | bool enable | void |
| set_draw_minimap | `<id>.setDrawMinimap(enabled)` | bool enabled | void |
| set_fit_content_height_enabled | `<id>.setFitContentHeightEnabled(enabled)` | bool enabled | void |
| set_fit_content_width_enabled | `<id>.setFitContentWidthEnabled(enabled)` | bool enabled | void |
| set_gutter_clickable | `<id>.setGutterClickable(gutter, clickable)` | int gutter, bool clickable | void |
| set_gutter_draw | `<id>.setGutterDraw(gutter, draw)` | int gutter, bool draw | void |
| set_gutter_name | `<id>.setGutterName(gutter, name)` | int gutter, string name | void |
| set_gutter_overwritable | `<id>.setGutterOverwritable(gutter, overwritable)` | int gutter, bool overwritable | void |
| set_gutter_type | `<id>.setGutterType(gutter, type)` | int gutter, int type | void |
| set_gutter_width | `<id>.setGutterWidth(gutter, width)` | int gutter, int width | void |
| set_h_scroll | `<id>.setHScroll(value)` | int value | void |
| set_line | `<id>.setLine(line, newText)` | int line, string newText | void |
| set_line_as_center_visible | `<id>.setLineAsCenterVisible(line, wrapIndex)` | int line, int wrapIndex | void |
| set_line_as_first_visible | `<id>.setLineAsFirstVisible(line, wrapIndex)` | int line, int wrapIndex | void |
| set_line_as_last_visible | `<id>.setLineAsLastVisible(line, wrapIndex)` | int line, int wrapIndex | void |
| set_line_background_color | `<id>.setLineBackgroundColor(line, color)` | int line, Color color | void |
| set_line_gutter_clickable | `<id>.setLineGutterClickable(line, gutter, clickable)` | int line, int gutter, bool clickable | void |
| set_line_gutter_item_color | `<id>.setLineGutterItemColor(line, gutter, color)` | int line, int gutter, Color color | void |
| set_line_gutter_text | `<id>.setLineGutterText(line, gutter, text)` | int line, int gutter, string text | void |
| set_line_wrapping_mode | `<id>.setLineWrappingMode(mode)` | int mode | void |
| set_move_caret_on_right_click_enabled | `<id>.setMoveCaretOnRightClickEnabled(enable)` | bool enable | void |
| set_multiple_carets_enabled | `<id>.setMultipleCaretsEnabled(enabled)` | bool enabled | void |
| set_overtype_mode_enabled | `<id>.setOvertypeModeEnabled(enabled)` | bool enabled | void |
| set_placeholder | `<id>.setPlaceholder(text)` | string text | void |
| set_scroll_past_end_of_file_enabled | `<id>.setScrollPastEndOfFileEnabled(enable)` | bool enable | void |
| set_search_flags | `<id>.setSearchFlags(flags)` | int flags | void |
| set_search_text | `<id>.setSearchText(searchText)` | string searchText | void |
| set_selection_mode | `<id>.setSelectionMode(mode)` | int mode | void |
| set_selection_origin_column | `<id>.setSelectionOriginColumn(column, caretIndex)` | int column, int caretIndex | void |
| set_selection_origin_line | `<id>.setSelectionOriginLine(line, canBeHidden, wrapIndex, caretIndex)` | int line, bool canBeHidden, int wrapIndex, int caretIndex | void |
| set_smooth_scroll_enabled | `<id>.setSmoothScrollEnabled(enable)` | bool enable | void |
| set_tab_size | `<id>.setTabSize(size)` | int size | void |
| set_v_scroll | `<id>.setVScroll(value)` | float value | void |
| set_v_scroll_speed | `<id>.setVScrollSpeed(speed)` | float speed | void |
| skip_selection_for_next_occurrence | `<id>.skipSelectionForNextOccurrence()` | - | void |
| start_action | `<id>.startAction(action)` | int action | void |
| swap_lines | `<id>.swapLines(fromLine, toLine)` | int fromLine, int toLine | void |
| tag_saved_version | `<id>.tagSavedVersion()` | - | void |
| undo | `<id>.undo()` | - | void |

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

