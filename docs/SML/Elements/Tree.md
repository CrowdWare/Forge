# Tree

## Inheritance

[Tree](Tree.md) → [Control](Control.md) → [CanvasItem](CanvasItem.md) → [Node](Node.md) → [Object](Object.md)

## Collection Items

This control appears to manage internal **items** (collection-style API).
Items are typically not represented as child nodes/properties in Godot.
In SML, this will be represented via **pseudo child elements** (documented per control).

## Properties

This page lists **only properties declared by `Tree`**.
Inherited properties are documented in: [Control](Control.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|
| - | showGuides | bool | true |
| - | rowHeight | int | - |
| - | indent | int | - |

## Events

This page lists **only signals declared by `Tree`**.
Inherited signals are documented in: [Control](Control.md)

| Godot Signal | SMS Event | Params |
|-|-|-|
| button_clicked | `on <id>.buttonClicked(item, column, id, mouseButtonIndex) { ... }` | Object item, int column, int id, int mouseButtonIndex |
| cell_selected | `on <id>.cellSelected() { ... }` | - |
| check_propagated_to_item | `on <id>.checkPropagatedToItem(item, column) { ... }` | Object item, int column |
| column_title_clicked | `on <id>.columnTitleClicked(column, mouseButtonIndex) { ... }` | int column, int mouseButtonIndex |
| custom_item_clicked | `on <id>.customItemClicked(mouseButtonIndex) { ... }` | int mouseButtonIndex |
| custom_popup_edited | `on <id>.customPopupEdited(arrowClicked) { ... }` | bool arrowClicked |
| empty_clicked | `on <id>.emptyClicked(clickPosition, mouseButtonIndex) { ... }` | Vector2 clickPosition, int mouseButtonIndex |
| item_activated | `on <id>.itemActivated() { ... }` | - |
| item_collapsed | `on <id>.itemCollapsed(item) { ... }` | Object item |
| item_edited | `on <id>.itemEdited() { ... }` | - |
| item_icon_double_clicked | `on <id>.itemIconDoubleClicked() { ... }` | - |
| item_mouse_selected | `on <id>.itemMouseSelected(mousePosition, mouseButtonIndex) { ... }` | Vector2 mousePosition, int mouseButtonIndex |
| item_selected | `on <id>.itemSelected() { ... }` | - |
| multi_selected | `on <id>.multiSelected(item, column, selected) { ... }` | Object item, int column, bool selected |
| nothing_selected | `on <id>.nothingSelected() { ... }` | - |

## Runtime Actions

This page lists **callable methods declared by `Tree`**.
Inherited actions are documented in: [Control](Control.md)

| Godot Method | SMS Call | Params | Returns |
|-|-|-|-|
| are_column_titles_visible | `<id>.areColumnTitlesVisible()` | - | bool |
| clear | `<id>.clear()` | - | void |
| deselect_all | `<id>.deselectAll()` | - | void |
| edit_selected | `<id>.editSelected(forceEdit)` | bool forceEdit | bool |
| ensure_cursor_is_visible | `<id>.ensureCursorIsVisible()` | - | void |
| get_button_id_at_position | `<id>.getButtonIdAtPosition(position)` | Vector2 position | int |
| get_column_at_position | `<id>.getColumnAtPosition(position)` | Vector2 position | int |
| get_column_expand_ratio | `<id>.getColumnExpandRatio(column)` | int column | int |
| get_column_title | `<id>.getColumnTitle(column)` | int column | string |
| get_column_title_alignment | `<id>.getColumnTitleAlignment(column)` | int column | int |
| get_column_title_direction | `<id>.getColumnTitleDirection(column)` | int column | int |
| get_column_title_language | `<id>.getColumnTitleLanguage(column)` | int column | string |
| get_column_title_tooltip_text | `<id>.getColumnTitleTooltipText(column)` | int column | string |
| get_column_width | `<id>.getColumnWidth(column)` | int column | int |
| get_custom_popup_rect | `<id>.getCustomPopupRect()` | - | Variant |
| get_drop_section_at_position | `<id>.getDropSectionAtPosition(position)` | Vector2 position | int |
| get_edited | `<id>.getEdited()` | - | Object |
| get_edited_column | `<id>.getEditedColumn()` | - | int |
| get_item_at_position | `<id>.getItemAtPosition(position)` | Vector2 position | Object |
| get_pressed_button | `<id>.getPressedButton()` | - | int |
| get_root | `<id>.getRoot()` | - | Object |
| get_scroll | `<id>.getScroll()` | - | Vector2 |
| get_selected | `<id>.getSelected()` | - | Object |
| get_selected_column | `<id>.getSelectedColumn()` | - | int |
| is_auto_tooltip_enabled | `<id>.isAutoTooltipEnabled()` | - | bool |
| is_column_clipping_content | `<id>.isColumnClippingContent(column)` | int column | bool |
| is_column_expanding | `<id>.isColumnExpanding(column)` | int column | bool |
| is_drag_unfolding_enabled | `<id>.isDragUnfoldingEnabled()` | - | bool |
| is_folding_hidden | `<id>.isFoldingHidden()` | - | bool |
| is_h_scroll_enabled | `<id>.isHScrollEnabled()` | - | bool |
| is_recursive_folding_enabled | `<id>.isRecursiveFoldingEnabled()` | - | bool |
| is_root_hidden | `<id>.isRootHidden()` | - | bool |
| is_scroll_hint_tiled | `<id>.isScrollHintTiled()` | - | bool |
| is_v_scroll_enabled | `<id>.isVScrollEnabled()` | - | bool |
| set_column_clip_content | `<id>.setColumnClipContent(column, enable)` | int column, bool enable | void |
| set_column_custom_minimum_width | `<id>.setColumnCustomMinimumWidth(column, minWidth)` | int column, int minWidth | void |
| set_column_expand | `<id>.setColumnExpand(column, expand)` | int column, bool expand | void |
| set_column_expand_ratio | `<id>.setColumnExpandRatio(column, ratio)` | int column, int ratio | void |
| set_column_title | `<id>.setColumnTitle(column, title)` | int column, string title | void |
| set_column_title_alignment | `<id>.setColumnTitleAlignment(column, titleAlignment)` | int column, int titleAlignment | void |
| set_column_title_direction | `<id>.setColumnTitleDirection(column, direction)` | int column, int direction | void |
| set_column_title_language | `<id>.setColumnTitleLanguage(column, language)` | int column, string language | void |
| set_column_title_tooltip_text | `<id>.setColumnTitleTooltipText(column, tooltipText)` | int column, string tooltipText | void |
| set_h_scroll_enabled | `<id>.setHScrollEnabled(hScroll)` | bool hScroll | void |
| set_v_scroll_enabled | `<id>.setVScrollEnabled(hScroll)` | bool hScroll | void |

## Actions

This page lists **only actions supported by the runtime** for `Tree`.
Inherited actions are documented in: [Control](Control.md)

| Action | SMS Call | Params | Returns |
|-|-|-|-|
| CreateRoot | `<id>.CreateRoot(text, path)` | string text, string path | int |
| CreateChild | `<id>.CreateChild(parentHandle, text, path, isDirectory)` | int parentHandle, string text, string path, bool isDirectory | int |
| AddButton | `<id>.AddButton(itemHandle, iconPath, buttonId, tooltip)` | int itemHandle, string iconPath, int buttonId, string tooltip | bool |
| GetSelectedPath | `<id>.GetSelectedPath()` | - | string |
| BindEvents | `<id>.BindEvents()` | - | void |

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


## SML Items (TODO)

This control appears to manage internal items, but a dedicated SML pseudo-child specification has not been defined yet.
Use generated runtime schema files (`ForgeRunner/Generated/Schema*.cs`) and this element reference as implementation hints.
