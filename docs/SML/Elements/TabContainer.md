# TabContainer

## Inheritance

[TabContainer](TabContainer.md) → [Container](Container.md) → [Control](Control.md) → [CanvasItem](CanvasItem.md) → [Node](Node.md) → [Object](Object.md)

## Collection Items

This control appears to manage internal **items** (collection-style API).
Items are typically not represented as child nodes/properties in Godot.
In SML, this will be represented via **pseudo child elements** (documented per control).

## Derived Classes

### Direct subclasses

- [ForgeDockingContainerControl](ForgeDockingContainerControl.md)

## Properties

This page lists **only properties declared by `TabContainer`**.
Inherited properties are documented in: [Container](Container.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|
| all_tabs_in_front | allTabsInFront | bool | - |
| clip_tabs | clipTabs | bool | - |
| current_tab | currentTab | int | - |
| deselect_enabled | deselectEnabled | bool | - |
| drag_to_rearrange_enabled | dragToRearrangeEnabled | bool | - |
| switch_on_drag_hover | switchOnDragHover | bool | - |
| tab_alignment | tabAlignment | int | - |
| tab_focus_mode | tabFocusMode | int | - |
| tabs_position | tabsPosition | int | - |
| tabs_rearrange_group | tabsRearrangeGroup | int | - |
| tabs_visible | tabsVisible | bool | - |
| use_hidden_tabs_for_min_size | useHiddenTabsForMinSize | bool | - |

## Events

This page lists **only signals declared by `TabContainer`**.
Inherited signals are documented in: [Container](Container.md)

| Godot Signal | SMS Event | Params |
|-|-|-|
| active_tab_rearranged | `on <id>.activeTabRearranged(idxTo) { ... }` | int idxTo |
| pre_popup_pressed | `on <id>.prePopupPressed() { ... }` | - |
| tab_button_pressed | `on <id>.tabButtonPressed(tab) { ... }` | int tab |
| tab_changed | `on <id>.tabChanged(tab) { ... }` | int tab |
| tab_clicked | `on <id>.tabClicked(tab) { ... }` | int tab |
| tab_hovered | `on <id>.tabHovered(tab) { ... }` | int tab |
| tab_selected | `on <id>.tabSelected(tab) { ... }` | int tab |

## Runtime Actions

This page lists **callable methods declared by `TabContainer`**.
Inherited actions are documented in: [Container](Container.md)

| Godot Method | SMS Call | Params | Returns |
|-|-|-|-|
| are_tabs_visible | `<id>.areTabsVisible()` | - | bool |
| get_current_tab_control | `<id>.getCurrentTabControl()` | - | Object |
| get_popup | `<id>.getPopup()` | - | Object |
| get_previous_tab | `<id>.getPreviousTab()` | - | int |
| get_tab_bar | `<id>.getTabBar()` | - | Object |
| get_tab_button_icon | `<id>.getTabButtonIcon(tabIdx)` | int tabIdx | Object |
| get_tab_control | `<id>.getTabControl(tabIdx)` | int tabIdx | Object |
| get_tab_count | `<id>.getTabCount()` | - | int |
| get_tab_icon | `<id>.getTabIcon(tabIdx)` | int tabIdx | Object |
| get_tab_icon_max_width | `<id>.getTabIconMaxWidth(tabIdx)` | int tabIdx | int |
| get_tab_idx_at_point | `<id>.getTabIdxAtPoint(point)` | Vector2 point | int |
| get_tab_metadata | `<id>.getTabMetadata(tabIdx)` | int tabIdx | void |
| get_tab_title | `<id>.getTabTitle(tabIdx)` | int tabIdx | string |
| get_tab_tooltip | `<id>.getTabTooltip(tabIdx)` | int tabIdx | string |
| is_tab_disabled | `<id>.isTabDisabled(tabIdx)` | int tabIdx | bool |
| is_tab_hidden | `<id>.isTabHidden(tabIdx)` | int tabIdx | bool |
| select_next_available | `<id>.selectNextAvailable()` | - | bool |
| select_previous_available | `<id>.selectPreviousAvailable()` | - | bool |
| set_tab_disabled | `<id>.setTabDisabled(tabIdx, disabled)` | int tabIdx, bool disabled | void |
| set_tab_hidden | `<id>.setTabHidden(tabIdx, hidden)` | int tabIdx, bool hidden | void |
| set_tab_icon_max_width | `<id>.setTabIconMaxWidth(tabIdx, width)` | int tabIdx, int width | void |
| set_tab_title | `<id>.setTabTitle(tabIdx, title)` | int tabIdx, string title | void |
| set_tab_tooltip | `<id>.setTabTooltip(tabIdx, tooltip)` | int tabIdx, string tooltip | void |

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

