# OptionButton

## Inheritance

[OptionButton](OptionButton.md) → [Button](Button.md) → [BaseButton](BaseButton.md) → [Control](Control.md) → [CanvasItem](CanvasItem.md) → [Node](Node.md) → [Object](Object.md)

## Collection Items

This control appears to manage internal **items** (collection-style API).
Items are typically not represented as child nodes/properties in Godot.
In SML, this will be represented via **pseudo child elements** (documented per control).

## Properties

This page lists **only properties declared by `OptionButton`**.
Inherited properties are documented in: [Button](Button.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|
| allow_reselect | allowReselect | bool | - |
| fit_to_longest_item | fitToLongestItem | bool | - |
| item_count | itemCount | int | - |
| selected | selected | int | - |

## Events

This page lists **only signals declared by `OptionButton`**.
Inherited signals are documented in: [Button](Button.md)

| Godot Signal | SMS Event | Params |
|-|-|-|
| item_focused | `on <id>.itemFocused(index) { ... }` | int index |
| item_selected | `on <id>.itemSelected(index) { ... }` | int index |

## Runtime Actions

This page lists **callable methods declared by `OptionButton`**.
Inherited actions are documented in: [Button](Button.md)

| Godot Method | SMS Call | Params | Returns |
|-|-|-|-|
| add_item | `<id>.addItem(label, id)` | string label, int id | void |
| add_separator | `<id>.addSeparator(text)` | string text | void |
| clear | `<id>.clear()` | - | void |
| get_item_auto_translate_mode | `<id>.getItemAutoTranslateMode(idx)` | int idx | int |
| get_item_icon | `<id>.getItemIcon(idx)` | int idx | Object |
| get_item_id | `<id>.getItemId(idx)` | int idx | int |
| get_item_index | `<id>.getItemIndex(id)` | int id | int |
| get_item_metadata | `<id>.getItemMetadata(idx)` | int idx | void |
| get_item_text | `<id>.getItemText(idx)` | int idx | string |
| get_item_tooltip | `<id>.getItemTooltip(idx)` | int idx | string |
| get_popup | `<id>.getPopup()` | - | Object |
| get_selectable_item | `<id>.getSelectableItem(fromLast)` | bool fromLast | int |
| get_selected_id | `<id>.getSelectedId()` | - | int |
| get_selected_metadata | `<id>.getSelectedMetadata()` | - | void |
| has_selectable_items | `<id>.hasSelectableItems()` | - | bool |
| is_item_disabled | `<id>.isItemDisabled(idx)` | int idx | bool |
| is_item_separator | `<id>.isItemSeparator(idx)` | int idx | bool |
| remove_item | `<id>.removeItem(idx)` | int idx | void |
| select | `<id>.select(idx)` | int idx | void |
| set_disable_shortcuts | `<id>.setDisableShortcuts(disabled)` | bool disabled | void |
| set_item_auto_translate_mode | `<id>.setItemAutoTranslateMode(idx, mode)` | int idx, int mode | void |
| set_item_disabled | `<id>.setItemDisabled(idx, disabled)` | int idx, bool disabled | void |
| set_item_id | `<id>.setItemId(idx, id)` | int idx, int id | void |
| set_item_text | `<id>.setItemText(idx, text)` | int idx, string text | void |
| set_item_tooltip | `<id>.setItemTooltip(idx, tooltip)` | int idx, string tooltip | void |
| show_popup | `<id>.showPopup()` | - | void |

## SML Items

`OptionButton` options are defined as **SML child nodes** (pseudo elements).
The runtime converts them to Godot OptionButton items internally.

### Supported item elements

- `Item`

### Item properties (SML)

| Property | Type | Default | Notes |
|-|-|-|-|
| id | identifier | - | Optional. Enables id-based event sugar (`on <id>.selected() { ... }`). |
| text | string | "" | Display text. |
| icon | string | "" | Optional icon resource/path. |
| disabled | bool | false | Disables the option. |
| selected | bool | false | If true, selects this option initially (first wins). |

### Example

```sml
OptionButton { id: quality
    Item { id: low; text: "Low" }
    Item { id: med; text: "Medium"; selected: true }
    Item { id: high; text: "High" }
}
```

### SMS Event Examples

```sms
// With explicit item ids:
on med.selected() { ... }

// Container fallback (index based):
on quality.itemSelected(index) { ... }
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

