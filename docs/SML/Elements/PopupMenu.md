# PopupMenu

## Inheritance

[PopupMenu](PopupMenu.md) → [Popup](Popup.md) → [Window](Window.md) → [Viewport](Viewport.md) → [Node](Node.md) → [Object](Object.md)

## Properties

This page lists **only properties declared by `PopupMenu`**.
Inherited properties are documented in: [Popup](Popup.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|
| allow_search | allowSearch | bool | - |
| hide_on_checkable_item_selection | hideOnCheckableItemSelection | bool | - |
| hide_on_item_selection | hideOnItemSelection | bool | - |
| hide_on_state_item_selection | hideOnStateItemSelection | bool | - |
| item_count | itemCount | int | - |
| prefer_native_menu | preferNativeMenu | bool | - |
| shrink_height | shrinkHeight | bool | - |
| shrink_width | shrinkWidth | bool | - |
| submenu_popup_delay | submenuPopupDelay | float | - |
| system_menu_id | systemMenuId | int | - |

## Events

This page lists **only signals declared by `PopupMenu`**.
Inherited signals are documented in: [Popup](Popup.md)

| Godot Signal | SMS Event | Params |
|-|-|-|
| id_focused | `on <id>.idFocused(id) { ... }` | int id |
| id_pressed | `on <id>.idPressed(id) { ... }` | int id |
| index_pressed | `on <id>.indexPressed(index) { ... }` | int index |
| menu_changed | `on <id>.menuChanged() { ... }` | - |

## Runtime Actions

This page lists **callable methods declared by `PopupMenu`**.
Inherited actions are documented in: [Popup](Popup.md)

| Godot Method | SMS Call | Params | Returns |
|-|-|-|-|
| add_check_item | `<id>.addCheckItem(label, id, accel)` | string label, int id, int accel | void |
| add_item | `<id>.addItem(label, id, accel)` | string label, int id, int accel | void |
| add_multistate_item | `<id>.addMultistateItem(label, maxStates, defaultState, id, accel)` | string label, int maxStates, int defaultState, int id, int accel | void |
| add_radio_check_item | `<id>.addRadioCheckItem(label, id, accel)` | string label, int id, int accel | void |
| add_separator | `<id>.addSeparator(label, id)` | string label, int id | void |
| add_submenu_item | `<id>.addSubmenuItem(label, submenu, id)` | string label, string submenu, int id | void |
| clear | `<id>.clear(freeSubmenus)` | bool freeSubmenus | void |
| get_focused_item | `<id>.getFocusedItem()` | - | int |
| get_item_accelerator | `<id>.getItemAccelerator(index)` | int index | int |
| get_item_auto_translate_mode | `<id>.getItemAutoTranslateMode(index)` | int index | int |
| get_item_icon | `<id>.getItemIcon(index)` | int index | Object |
| get_item_icon_max_width | `<id>.getItemIconMaxWidth(index)` | int index | int |
| get_item_icon_modulate | `<id>.getItemIconModulate(index)` | int index | Color |
| get_item_id | `<id>.getItemId(index)` | int index | int |
| get_item_indent | `<id>.getItemIndent(index)` | int index | int |
| get_item_index | `<id>.getItemIndex(id)` | int id | int |
| get_item_language | `<id>.getItemLanguage(index)` | int index | string |
| get_item_metadata | `<id>.getItemMetadata(index)` | int index | void |
| get_item_multistate | `<id>.getItemMultistate(index)` | int index | int |
| get_item_multistate_max | `<id>.getItemMultistateMax(index)` | int index | int |
| get_item_shortcut | `<id>.getItemShortcut(index)` | int index | Object |
| get_item_submenu | `<id>.getItemSubmenu(index)` | int index | string |
| get_item_submenu_node | `<id>.getItemSubmenuNode(index)` | int index | Object |
| get_item_text | `<id>.getItemText(index)` | int index | string |
| get_item_text_direction | `<id>.getItemTextDirection(index)` | int index | int |
| get_item_tooltip | `<id>.getItemTooltip(index)` | int index | string |
| get_system_menu | `<id>.getSystemMenu()` | - | int |
| is_item_checkable | `<id>.isItemCheckable(index)` | int index | bool |
| is_item_checked | `<id>.isItemChecked(index)` | int index | bool |
| is_item_disabled | `<id>.isItemDisabled(index)` | int index | bool |
| is_item_radio_checkable | `<id>.isItemRadioCheckable(index)` | int index | bool |
| is_item_separator | `<id>.isItemSeparator(index)` | int index | bool |
| is_item_shortcut_disabled | `<id>.isItemShortcutDisabled(index)` | int index | bool |
| is_native_menu | `<id>.isNativeMenu()` | - | bool |
| is_system_menu | `<id>.isSystemMenu()` | - | bool |
| remove_item | `<id>.removeItem(index)` | int index | void |
| scroll_to_item | `<id>.scrollToItem(index)` | int index | void |
| set_focused_item | `<id>.setFocusedItem(index)` | int index | void |
| set_item_accelerator | `<id>.setItemAccelerator(index, accel)` | int index, int accel | void |
| set_item_as_checkable | `<id>.setItemAsCheckable(index, enable)` | int index, bool enable | void |
| set_item_as_radio_checkable | `<id>.setItemAsRadioCheckable(index, enable)` | int index, bool enable | void |
| set_item_as_separator | `<id>.setItemAsSeparator(index, enable)` | int index, bool enable | void |
| set_item_auto_translate_mode | `<id>.setItemAutoTranslateMode(index, mode)` | int index, int mode | void |
| set_item_checked | `<id>.setItemChecked(index, checked)` | int index, bool checked | void |
| set_item_disabled | `<id>.setItemDisabled(index, disabled)` | int index, bool disabled | void |
| set_item_icon_max_width | `<id>.setItemIconMaxWidth(index, width)` | int index, int width | void |
| set_item_icon_modulate | `<id>.setItemIconModulate(index, modulate)` | int index, Color modulate | void |
| set_item_id | `<id>.setItemId(index, id)` | int index, int id | void |
| set_item_indent | `<id>.setItemIndent(index, indent)` | int index, int indent | void |
| set_item_language | `<id>.setItemLanguage(index, language)` | int index, string language | void |
| set_item_multistate | `<id>.setItemMultistate(index, state)` | int index, int state | void |
| set_item_multistate_max | `<id>.setItemMultistateMax(index, maxStates)` | int index, int maxStates | void |
| set_item_shortcut_disabled | `<id>.setItemShortcutDisabled(index, disabled)` | int index, bool disabled | void |
| set_item_submenu | `<id>.setItemSubmenu(index, submenu)` | int index, string submenu | void |
| set_item_text | `<id>.setItemText(index, text)` | int index, string text | void |
| set_item_text_direction | `<id>.setItemTextDirection(index, direction)` | int index, int direction | void |
| set_item_tooltip | `<id>.setItemTooltip(index, tooltip)` | int index, string tooltip | void |
| set_system_menu | `<id>.setSystemMenu(systemMenuId)` | int systemMenuId | void |
| toggle_item_checked | `<id>.toggleItemChecked(index)` | int index | void |
| toggle_item_multistate | `<id>.toggleItemMultistate(index)` | int index | void |

## SML Items

`PopupMenu` items are defined as **SML child nodes** (pseudo elements).
The runtime converts them to Godot menu items internally.

### Supported item elements

- `Item`
- `CheckItem`
- `Separator`

### Item properties (SML)

| Property | Type | Default | Notes |
|-|-|-|-|
| id | identifier | - | Optional. Enables id-based event sugar (`on <id>.clicked() { ... }`). |
| text | string | "" | Display text. |
| checked | bool | false | Only for `CheckItem`. |
| disabled | bool | false | Optional. |

### Example

```sml
PopupMenu { id: fileMenu
    Item { id: open; text: "Open" }
    Item { id: save; text: "Save" }
    Separator { }
    CheckItem { id: autosave; text: "Auto Save"; checked: true }
}
```

### SMS Event Examples

```sms
// With explicit item ids:
on open.clicked() { ... }
on autosave.clicked() { ... }

// Without item ids (container fallback):
on fileMenu.idPressed(id) { ... }
```
