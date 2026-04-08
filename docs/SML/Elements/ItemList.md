# ItemList

## Inheritance

[ItemList](ItemList.md) → [Control](Control.md) → [CanvasItem](CanvasItem.md) → [Node](Node.md) → [Object](Object.md)

## Collection Items

This control appears to manage internal **items** (collection-style API).
Items are typically not represented as child nodes/properties in Godot.
In SML, this will be represented via **pseudo child elements** (documented per control).

## Properties

This page lists **only properties declared by `ItemList`**.
Inherited properties are documented in: [Control](Control.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|
| allow_reselect | allowReselect | bool | - |
| allow_rmb_select | allowRmbSelect | bool | - |
| allow_search | allowSearch | bool | - |
| auto_height | autoHeight | bool | - |
| auto_width | autoWidth | bool | - |
| fixed_column_width | fixedColumnWidth | int | - |
| icon_mode | iconMode | int | - |
| icon_scale | iconScale | float | - |
| item_count | itemCount | int | - |
| max_columns | maxColumns | int | - |
| max_text_lines | maxTextLines | int | - |
| same_column_width | sameColumnWidth | bool | - |
| scroll_hint_mode | scrollHintMode | int | - |
| select_mode | selectMode | int | - |
| text_overrun_behavior | textOverrunBehavior | int | - |
| tile_scroll_hint | tileScrollHint | bool | - |
| wraparound_items | wraparoundItems | bool | - |

## Events

This page lists **only signals declared by `ItemList`**.
Inherited signals are documented in: [Control](Control.md)

| Godot Signal | SMS Event | Params |
|-|-|-|
| empty_clicked | `on <id>.emptyClicked(atPosition, mouseButtonIndex) { ... }` | Vector2 atPosition, int mouseButtonIndex |
| item_activated | `on <id>.itemActivated(index) { ... }` | int index |
| item_clicked | `on <id>.itemClicked(index, atPosition, mouseButtonIndex) { ... }` | int index, Vector2 atPosition, int mouseButtonIndex |
| item_selected | `on <id>.itemSelected(index) { ... }` | int index |
| multi_selected | `on <id>.multiSelected(index, selected) { ... }` | int index, bool selected |

## Runtime Actions

This page lists **callable methods declared by `ItemList`**.
Inherited actions are documented in: [Control](Control.md)

| Godot Method | SMS Call | Params | Returns |
|-|-|-|-|
| clear | `<id>.clear()` | - | void |
| deselect | `<id>.deselect(idx)` | int idx | void |
| deselect_all | `<id>.deselectAll()` | - | void |
| ensure_current_is_visible | `<id>.ensureCurrentIsVisible()` | - | void |
| force_update_list_size | `<id>.forceUpdateListSize()` | - | void |
| get_h_scroll_bar | `<id>.getHScrollBar()` | - | Object |
| get_item_at_position | `<id>.getItemAtPosition(position, exact)` | Vector2 position, bool exact | int |
| get_item_auto_translate_mode | `<id>.getItemAutoTranslateMode(idx)` | int idx | int |
| get_item_custom_bg_color | `<id>.getItemCustomBgColor(idx)` | int idx | Color |
| get_item_custom_fg_color | `<id>.getItemCustomFgColor(idx)` | int idx | Color |
| get_item_icon | `<id>.getItemIcon(idx)` | int idx | Object |
| get_item_icon_modulate | `<id>.getItemIconModulate(idx)` | int idx | Color |
| get_item_icon_region | `<id>.getItemIconRegion(idx)` | int idx | Variant |
| get_item_language | `<id>.getItemLanguage(idx)` | int idx | string |
| get_item_metadata | `<id>.getItemMetadata(idx)` | int idx | void |
| get_item_rect | `<id>.getItemRect(idx, expand)` | int idx, bool expand | Variant |
| get_item_text | `<id>.getItemText(idx)` | int idx | string |
| get_item_text_direction | `<id>.getItemTextDirection(idx)` | int idx | int |
| get_item_tooltip | `<id>.getItemTooltip(idx)` | int idx | string |
| get_selected_items | `<id>.getSelectedItems()` | - | Variant |
| get_v_scroll_bar | `<id>.getVScrollBar()` | - | Object |
| has_auto_height | `<id>.hasAutoHeight()` | - | bool |
| has_auto_width | `<id>.hasAutoWidth()` | - | bool |
| has_wraparound_items | `<id>.hasWraparoundItems()` | - | bool |
| is_anything_selected | `<id>.isAnythingSelected()` | - | bool |
| is_item_disabled | `<id>.isItemDisabled(idx)` | int idx | bool |
| is_item_icon_transposed | `<id>.isItemIconTransposed(idx)` | int idx | bool |
| is_item_selectable | `<id>.isItemSelectable(idx)` | int idx | bool |
| is_item_tooltip_enabled | `<id>.isItemTooltipEnabled(idx)` | int idx | bool |
| is_scroll_hint_tiled | `<id>.isScrollHintTiled()` | - | bool |
| is_selected | `<id>.isSelected(idx)` | int idx | bool |
| move_item | `<id>.moveItem(fromIdx, toIdx)` | int fromIdx, int toIdx | void |
| remove_item | `<id>.removeItem(idx)` | int idx | void |
| select | `<id>.select(idx, single)` | int idx, bool single | void |
| set_item_auto_translate_mode | `<id>.setItemAutoTranslateMode(idx, mode)` | int idx, int mode | void |
| set_item_custom_bg_color | `<id>.setItemCustomBgColor(idx, customBgColor)` | int idx, Color customBgColor | void |
| set_item_custom_fg_color | `<id>.setItemCustomFgColor(idx, customFgColor)` | int idx, Color customFgColor | void |
| set_item_disabled | `<id>.setItemDisabled(idx, disabled)` | int idx, bool disabled | void |
| set_item_icon_modulate | `<id>.setItemIconModulate(idx, modulate)` | int idx, Color modulate | void |
| set_item_icon_transposed | `<id>.setItemIconTransposed(idx, transposed)` | int idx, bool transposed | void |
| set_item_language | `<id>.setItemLanguage(idx, language)` | int idx, string language | void |
| set_item_selectable | `<id>.setItemSelectable(idx, selectable)` | int idx, bool selectable | void |
| set_item_text | `<id>.setItemText(idx, text)` | int idx, string text | void |
| set_item_text_direction | `<id>.setItemTextDirection(idx, direction)` | int idx, int direction | void |
| set_item_tooltip | `<id>.setItemTooltip(idx, tooltip)` | int idx, string tooltip | void |
| set_item_tooltip_enabled | `<id>.setItemTooltipEnabled(idx, enable)` | int idx, bool enable | void |
| sort_items_by_text | `<id>.sortItemsByText()` | - | void |

## SML Items

`ItemList` entries are defined as **SML child nodes** (pseudo elements).
The runtime converts them to Godot ItemList items internally.

### Supported item elements

- `Item`

### Item properties (SML)

| Property | Type | Default | Notes |
|-|-|-|-|
| id | identifier | - | Optional. Enables id-based event sugar (`on <id>.selected() { ... }`). |
| text | string | "" | Display text. |
| icon | string | "" | Optional icon resource/path. |
| selected | bool | false | Initial selection state (single-select). |
| disabled | bool | false | Disables the item. |
| tooltip | string | "" | Optional tooltip text. |

### Example

```sml
ItemList { id: files
    Item { id: a; text: "Readme.md"; icon: "res:/icons/doc.svg" }
    Item { id: b; text: "Todo.md" }
    Item { text: "Disabled item"; disabled: true }
}
```

### SMS Event Examples

```sms
// With explicit item ids:
on a.selected() { ... }

// Without item ids (container fallback):
on files.itemSelected(index) { ... }
```

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

