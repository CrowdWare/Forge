# CodeEdit

## Inheritance

[CodeEdit](CodeEdit.md) → [TextEdit](TextEdit.md) → [Control](Control.md) → [CanvasItem](CanvasItem.md) → [Node](Node.md) → [Object](Object.md)

## Properties

This page lists **only properties declared by `CodeEdit`**.
Inherited properties are documented in: [TextEdit](TextEdit.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|
| auto_brace_completion_enabled | autoBraceCompletionEnabled | bool | - |
| auto_brace_completion_highlight_matching | autoBraceCompletionHighlightMatching | bool | - |
| code_completion_enabled | codeCompletionEnabled | bool | - |
| gutters_draw_bookmarks | guttersDrawBookmarks | bool | - |
| gutters_draw_breakpoints_gutter | guttersDrawBreakpointsGutter | bool | - |
| gutters_draw_executing_lines | guttersDrawExecutingLines | bool | - |
| gutters_draw_fold_gutter | guttersDrawFoldGutter | bool | - |
| gutters_draw_line_numbers | guttersDrawLineNumbers | bool | - |
| gutters_line_numbers_min_digits | guttersLineNumbersMinDigits | int | - |
| gutters_zero_pad_line_numbers | guttersZeroPadLineNumbers | bool | - |
| indent_automatic | indentAutomatic | bool | - |
| indent_size | indentSize | int | - |
| indent_use_spaces | indentUseSpaces | bool | - |
| line_folding | lineFolding | bool | - |
| symbol_lookup_on_click | symbolLookupOnClick | bool | - |
| symbol_tooltip_on_hover | symbolTooltipOnHover | bool | - |

## Events

This page lists **only signals declared by `CodeEdit`**.
Inherited signals are documented in: [TextEdit](TextEdit.md)

| Godot Signal | SMS Event | Params |
|-|-|-|
| breakpoint_toggled | `on <id>.breakpointToggled(line) { ... }` | int line |
| code_completion_requested | `on <id>.codeCompletionRequested() { ... }` | - |
| symbol_hovered | `on <id>.symbolHovered(symbol, line, column) { ... }` | string symbol, int line, int column |
| symbol_lookup | `on <id>.symbolLookup(symbol, line, column) { ... }` | string symbol, int line, int column |
| symbol_validate | `on <id>.symbolValidate(symbol) { ... }` | string symbol |

## Runtime Actions

This page lists **callable methods declared by `CodeEdit`**.
Inherited actions are documented in: [TextEdit](TextEdit.md)

| Godot Method | SMS Call | Params | Returns |
|-|-|-|-|
| add_auto_brace_completion_pair | `<id>.addAutoBraceCompletionPair(startKey, endKey)` | string startKey, string endKey | void |
| add_comment_delimiter | `<id>.addCommentDelimiter(startKey, endKey, lineOnly)` | string startKey, string endKey, bool lineOnly | void |
| add_string_delimiter | `<id>.addStringDelimiter(startKey, endKey, lineOnly)` | string startKey, string endKey, bool lineOnly | void |
| can_fold_line | `<id>.canFoldLine(line)` | int line | bool |
| cancel_code_completion | `<id>.cancelCodeCompletion()` | - | void |
| clear_bookmarked_lines | `<id>.clearBookmarkedLines()` | - | void |
| clear_breakpointed_lines | `<id>.clearBreakpointedLines()` | - | void |
| clear_comment_delimiters | `<id>.clearCommentDelimiters()` | - | void |
| clear_executing_lines | `<id>.clearExecutingLines()` | - | void |
| clear_string_delimiters | `<id>.clearStringDelimiters()` | - | void |
| confirm_code_completion | `<id>.confirmCodeCompletion(replace)` | bool replace | void |
| convert_indent | `<id>.convertIndent(fromLine, toLine)` | int fromLine, int toLine | void |
| create_code_region | `<id>.createCodeRegion()` | - | void |
| delete_lines | `<id>.deleteLines()` | - | void |
| do_indent | `<id>.doIndent()` | - | void |
| duplicate_lines | `<id>.duplicateLines()` | - | void |
| duplicate_selection | `<id>.duplicateSelection()` | - | void |
| fold_all_lines | `<id>.foldAllLines()` | - | void |
| fold_line | `<id>.foldLine(line)` | int line | void |
| get_auto_brace_completion_close_key | `<id>.getAutoBraceCompletionCloseKey(openKey)` | string openKey | string |
| get_auto_indent_prefixes | `<id>.getAutoIndentPrefixes()` | - | Variant |
| get_bookmarked_lines | `<id>.getBookmarkedLines()` | - | Variant |
| get_breakpointed_lines | `<id>.getBreakpointedLines()` | - | Variant |
| get_code_completion_option | `<id>.getCodeCompletionOption(index)` | int index | Variant |
| get_code_completion_options | `<id>.getCodeCompletionOptions()` | - | Variant |
| get_code_completion_selected_index | `<id>.getCodeCompletionSelectedIndex()` | - | int |
| get_code_region_end_tag | `<id>.getCodeRegionEndTag()` | - | string |
| get_code_region_start_tag | `<id>.getCodeRegionStartTag()` | - | string |
| get_comment_delimiters | `<id>.getCommentDelimiters()` | - | Variant |
| get_delimiter_end_key | `<id>.getDelimiterEndKey(delimiterIndex)` | int delimiterIndex | string |
| get_delimiter_end_position | `<id>.getDelimiterEndPosition(line, column)` | int line, int column | Vector2 |
| get_delimiter_start_key | `<id>.getDelimiterStartKey(delimiterIndex)` | int delimiterIndex | string |
| get_delimiter_start_position | `<id>.getDelimiterStartPosition(line, column)` | int line, int column | Vector2 |
| get_executing_lines | `<id>.getExecutingLines()` | - | Variant |
| get_folded_lines | `<id>.getFoldedLines()` | - | Variant |
| get_line_numbers_min_digits | `<id>.getLineNumbersMinDigits()` | - | int |
| get_string_delimiters | `<id>.getStringDelimiters()` | - | Variant |
| get_text_for_code_completion | `<id>.getTextForCodeCompletion()` | - | string |
| get_text_for_symbol_lookup | `<id>.getTextForSymbolLookup()` | - | string |
| get_text_with_cursor_char | `<id>.getTextWithCursorChar(line, column)` | int line, int column | string |
| has_auto_brace_completion_close_key | `<id>.hasAutoBraceCompletionCloseKey(closeKey)` | string closeKey | bool |
| has_auto_brace_completion_open_key | `<id>.hasAutoBraceCompletionOpenKey(openKey)` | string openKey | bool |
| has_comment_delimiter | `<id>.hasCommentDelimiter(startKey)` | string startKey | bool |
| has_string_delimiter | `<id>.hasStringDelimiter(startKey)` | string startKey | bool |
| indent_lines | `<id>.indentLines()` | - | void |
| is_auto_indent_enabled | `<id>.isAutoIndentEnabled()` | - | bool |
| is_draw_line_numbers_enabled | `<id>.isDrawLineNumbersEnabled()` | - | bool |
| is_drawing_bookmarks_gutter | `<id>.isDrawingBookmarksGutter()` | - | bool |
| is_drawing_breakpoints_gutter | `<id>.isDrawingBreakpointsGutter()` | - | bool |
| is_drawing_executing_lines_gutter | `<id>.isDrawingExecutingLinesGutter()` | - | bool |
| is_drawing_fold_gutter | `<id>.isDrawingFoldGutter()` | - | bool |
| is_highlight_matching_braces_enabled | `<id>.isHighlightMatchingBracesEnabled()` | - | bool |
| is_in_comment | `<id>.isInComment(line, column)` | int line, int column | int |
| is_in_string | `<id>.isInString(line, column)` | int line, int column | int |
| is_indent_using_spaces | `<id>.isIndentUsingSpaces()` | - | bool |
| is_line_bookmarked | `<id>.isLineBookmarked(line)` | int line | bool |
| is_line_breakpointed | `<id>.isLineBreakpointed(line)` | int line | bool |
| is_line_code_region_end | `<id>.isLineCodeRegionEnd(line)` | int line | bool |
| is_line_code_region_start | `<id>.isLineCodeRegionStart(line)` | int line | bool |
| is_line_executing | `<id>.isLineExecuting(line)` | int line | bool |
| is_line_folded | `<id>.isLineFolded(line)` | int line | bool |
| is_line_folding_enabled | `<id>.isLineFoldingEnabled()` | - | bool |
| is_line_numbers_zero_padded | `<id>.isLineNumbersZeroPadded()` | - | bool |
| is_symbol_lookup_on_click_enabled | `<id>.isSymbolLookupOnClickEnabled()` | - | bool |
| is_symbol_tooltip_on_hover_enabled | `<id>.isSymbolTooltipOnHoverEnabled()` | - | bool |
| move_lines_down | `<id>.moveLinesDown()` | - | void |
| move_lines_up | `<id>.moveLinesUp()` | - | void |
| remove_comment_delimiter | `<id>.removeCommentDelimiter(startKey)` | string startKey | void |
| remove_string_delimiter | `<id>.removeStringDelimiter(startKey)` | string startKey | void |
| request_code_completion | `<id>.requestCodeCompletion(force)` | bool force | void |
| set_auto_indent_enabled | `<id>.setAutoIndentEnabled(enable)` | bool enable | void |
| set_code_completion_selected_index | `<id>.setCodeCompletionSelectedIndex(index)` | int index | void |
| set_code_hint | `<id>.setCodeHint(codeHint)` | string codeHint | void |
| set_code_hint_draw_below | `<id>.setCodeHintDrawBelow(drawBelow)` | bool drawBelow | void |
| set_code_region_tags | `<id>.setCodeRegionTags(start, end)` | string start, string end | void |
| set_draw_bookmarks_gutter | `<id>.setDrawBookmarksGutter(enable)` | bool enable | void |
| set_draw_breakpoints_gutter | `<id>.setDrawBreakpointsGutter(enable)` | bool enable | void |
| set_draw_executing_lines_gutter | `<id>.setDrawExecutingLinesGutter(enable)` | bool enable | void |
| set_draw_fold_gutter | `<id>.setDrawFoldGutter(enable)` | bool enable | void |
| set_draw_line_numbers | `<id>.setDrawLineNumbers(enable)` | bool enable | void |
| set_highlight_matching_braces_enabled | `<id>.setHighlightMatchingBracesEnabled(enable)` | bool enable | void |
| set_indent_using_spaces | `<id>.setIndentUsingSpaces(useSpaces)` | bool useSpaces | void |
| set_line_as_bookmarked | `<id>.setLineAsBookmarked(line, bookmarked)` | int line, bool bookmarked | void |
| set_line_as_breakpoint | `<id>.setLineAsBreakpoint(line, breakpointed)` | int line, bool breakpointed | void |
| set_line_as_executing | `<id>.setLineAsExecuting(line, executing)` | int line, bool executing | void |
| set_line_folding_enabled | `<id>.setLineFoldingEnabled(enabled)` | bool enabled | void |
| set_line_numbers_min_digits | `<id>.setLineNumbersMinDigits(count)` | int count | void |
| set_line_numbers_zero_padded | `<id>.setLineNumbersZeroPadded(enable)` | bool enable | void |
| set_symbol_lookup_on_click_enabled | `<id>.setSymbolLookupOnClickEnabled(enable)` | bool enable | void |
| set_symbol_lookup_word_as_valid | `<id>.setSymbolLookupWordAsValid(valid)` | bool valid | void |
| set_symbol_tooltip_on_hover_enabled | `<id>.setSymbolTooltipOnHoverEnabled(enable)` | bool enable | void |
| toggle_foldable_line | `<id>.toggleFoldableLine(line)` | int line | void |
| toggle_foldable_lines_at_carets | `<id>.toggleFoldableLinesAtCarets()` | - | void |
| unfold_all_lines | `<id>.unfoldAllLines()` | - | void |
| unfold_line | `<id>.unfoldLine(line)` | int line | void |
| unindent_lines | `<id>.unindentLines()` | - | void |
| update_code_completion_options | `<id>.updateCodeCompletionOptions(force)` | bool force | void |

## Actions

This page lists **only actions supported by the runtime** for `CodeEdit`.
Inherited actions are documented in: [TextEdit](TextEdit.md)

| Action | SMS Call | Params | Returns |
|-|-|-|-|
| onSave | `<id>.onSave(callbackName)` | string callbackName | void |

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

