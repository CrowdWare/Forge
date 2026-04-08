# TabBar

## Inheritance

[TabBar](TabBar.md) → [Control](Control.md) → [CanvasItem](CanvasItem.md) → [Node](Node.md) → [Object](Object.md)

## Collection Items

This control appears to manage internal **items** (collection-style API).
Items are typically not represented as child nodes/properties in Godot.
In SML, this will be represented via **pseudo child elements** (documented per control).

## Properties

This page lists **only properties declared by `TabBar`**.
Inherited properties are documented in: [Control](Control.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|
| clip_tabs | clipTabs | bool | - |
| close_with_middle_mouse | closeWithMiddleMouse | bool | - |
| current_tab | currentTab | int | - |
| deselect_enabled | deselectEnabled | bool | - |
| drag_to_rearrange_enabled | dragToRearrangeEnabled | bool | - |
| max_tab_width | maxTabWidth | int | - |
| scroll_to_selected | scrollToSelected | bool | - |
| scrolling_enabled | scrollingEnabled | bool | - |
| select_with_rmb | selectWithRmb | bool | - |
| switch_on_drag_hover | switchOnDragHover | bool | - |
| tab_alignment | tabAlignment | int | - |
| tab_close_display_policy | tabCloseDisplayPolicy | int | - |
| tab_count | tabCount | int | - |
| tabs_rearrange_group | tabsRearrangeGroup | int | - |

## Events

This page lists **only signals declared by `TabBar`**.
Inherited signals are documented in: [Control](Control.md)

| Godot Signal | SMS Event | Params |
|-|-|-|
| active_tab_rearranged | `on <id>.activeTabRearranged(idxTo) { ... }` | int idxTo |
| tab_button_pressed | `on <id>.tabButtonPressed(tab) { ... }` | int tab |
| tab_changed | `on <id>.tabChanged(tab) { ... }` | int tab |
| tab_clicked | `on <id>.tabClicked(tab) { ... }` | int tab |
| tab_close_pressed | `on <id>.tabClosePressed(tab) { ... }` | int tab |
| tab_hovered | `on <id>.tabHovered(tab) { ... }` | int tab |
| tab_rmb_clicked | `on <id>.tabRmbClicked(tab) { ... }` | int tab |
| tab_selected | `on <id>.tabSelected(tab) { ... }` | int tab |

## Runtime Actions

This page lists **callable methods declared by `TabBar`**.
Inherited actions are documented in: [Control](Control.md)

| Godot Method | SMS Call | Params | Returns |
|-|-|-|-|
| clear_tabs | `<id>.clearTabs()` | - | void |
| ensure_tab_visible | `<id>.ensureTabVisible(idx)` | int idx | void |
| get_offset_buttons_visible | `<id>.getOffsetButtonsVisible()` | - | bool |
| get_previous_tab | `<id>.getPreviousTab()` | - | int |
| get_tab_button_icon | `<id>.getTabButtonIcon(tabIdx)` | int tabIdx | Object |
| get_tab_icon | `<id>.getTabIcon(tabIdx)` | int tabIdx | Object |
| get_tab_icon_max_width | `<id>.getTabIconMaxWidth(tabIdx)` | int tabIdx | int |
| get_tab_idx_at_point | `<id>.getTabIdxAtPoint(point)` | Vector2 point | int |
| get_tab_language | `<id>.getTabLanguage(tabIdx)` | int tabIdx | string |
| get_tab_metadata | `<id>.getTabMetadata(tabIdx)` | int tabIdx | void |
| get_tab_offset | `<id>.getTabOffset()` | - | int |
| get_tab_rect | `<id>.getTabRect(tabIdx)` | int tabIdx | Variant |
| get_tab_text_direction | `<id>.getTabTextDirection(tabIdx)` | int tabIdx | int |
| get_tab_title | `<id>.getTabTitle(tabIdx)` | int tabIdx | string |
| get_tab_tooltip | `<id>.getTabTooltip(tabIdx)` | int tabIdx | string |
| is_tab_disabled | `<id>.isTabDisabled(tabIdx)` | int tabIdx | bool |
| is_tab_hidden | `<id>.isTabHidden(tabIdx)` | int tabIdx | bool |
| move_tab | `<id>.moveTab(from, to)` | int from, int to | void |
| remove_tab | `<id>.removeTab(tabIdx)` | int tabIdx | void |
| select_next_available | `<id>.selectNextAvailable()` | - | bool |
| select_previous_available | `<id>.selectPreviousAvailable()` | - | bool |
| set_tab_disabled | `<id>.setTabDisabled(tabIdx, disabled)` | int tabIdx, bool disabled | void |
| set_tab_hidden | `<id>.setTabHidden(tabIdx, hidden)` | int tabIdx, bool hidden | void |
| set_tab_icon_max_width | `<id>.setTabIconMaxWidth(tabIdx, width)` | int tabIdx, int width | void |
| set_tab_language | `<id>.setTabLanguage(tabIdx, language)` | int tabIdx, string language | void |
| set_tab_text_direction | `<id>.setTabTextDirection(tabIdx, direction)` | int tabIdx, int direction | void |
| set_tab_title | `<id>.setTabTitle(tabIdx, title)` | int tabIdx, string title | void |
| set_tab_tooltip | `<id>.setTabTooltip(tabIdx, tooltip)` | int tabIdx, string tooltip | void |

## SML Tabs

`TabBar` tabs are defined as **SML child nodes** (pseudo elements).
The runtime converts them to Godot TabBar tabs internally.

### Supported tab elements

- `Tab`

### Tab properties (SML)

| Property | Type | Default | Notes |
|-|-|-|-|
| id | identifier | - | Optional. Enables id-based event sugar (`on <id>.tabSelected() { ... }`). |
| title | string | "" | Tab title. |
| icon | string | "" | Optional icon resource/path. |
| disabled | bool | false | Disables selecting the tab. |
| hidden | bool | false | Hides the tab. |
| selected | bool | false | If true, selects this tab initially (first wins). |

### Example

```sml
TabBar { id: tabs
    Tab { id: home; title: "Home"; selected: true }
    Tab { id: settings; title: "Settings" }
}
```

### SMS Event Examples

```sms
// With explicit tab ids:
on home.tabSelected() { ... }

// Container fallback (index based):
on tabs.tabChanged(index) { ... }
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

