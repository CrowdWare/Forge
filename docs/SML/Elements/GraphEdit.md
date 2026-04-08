# GraphEdit

## Inheritance

[GraphEdit](GraphEdit.md) → [Control](Control.md) → [CanvasItem](CanvasItem.md) → [Node](Node.md) → [Object](Object.md)

## Properties

This page lists **only properties declared by `GraphEdit`**.
Inherited properties are documented in: [Control](Control.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|
| connection_lines_antialiased | connectionLinesAntialiased | bool | - |
| connection_lines_curvature | connectionLinesCurvature | float | - |
| connection_lines_thickness | connectionLinesThickness | float | - |
| grid_pattern | gridPattern | int | - |
| minimap_enabled | minimapEnabled | bool | - |
| minimap_opacity | minimapOpacity | float | - |
| minimap_size | minimapSize | Vector2 | - |
| panning_scheme | panningScheme | int | - |
| right_disconnects | rightDisconnects | bool | - |
| scroll_offset | scrollOffset | Vector2 | - |
| show_arrange_button | showArrangeButton | bool | - |
| show_grid | showGrid | bool | - |
| show_grid_buttons | showGridButtons | bool | - |
| show_menu | showMenu | bool | - |
| show_minimap_button | showMinimapButton | bool | - |
| show_zoom_buttons | showZoomButtons | bool | - |
| show_zoom_label | showZoomLabel | bool | - |
| snapping_distance | snappingDistance | int | - |
| snapping_enabled | snappingEnabled | bool | - |
| zoom | zoom | float | - |
| zoom_max | zoomMax | float | - |
| zoom_min | zoomMin | float | - |
| zoom_step | zoomStep | float | - |

## Events

This page lists **only signals declared by `GraphEdit`**.
Inherited signals are documented in: [Control](Control.md)

| Godot Signal | SMS Event | Params |
|-|-|-|
| begin_node_move | `on <id>.beginNodeMove() { ... }` | - |
| connection_drag_ended | `on <id>.connectionDragEnded() { ... }` | - |
| connection_drag_started | `on <id>.connectionDragStarted(fromNode, fromPort, isOutput) { ... }` | Variant fromNode, int fromPort, bool isOutput |
| connection_from_empty | `on <id>.connectionFromEmpty(toNode, toPort, releasePosition) { ... }` | Variant toNode, int toPort, Vector2 releasePosition |
| connection_request | `on <id>.connectionRequest(fromNode, fromPort, toNode, toPort) { ... }` | Variant fromNode, int fromPort, Variant toNode, int toPort |
| connection_to_empty | `on <id>.connectionToEmpty(fromNode, fromPort, releasePosition) { ... }` | Variant fromNode, int fromPort, Vector2 releasePosition |
| copy_nodes_request | `on <id>.copyNodesRequest() { ... }` | - |
| cut_nodes_request | `on <id>.cutNodesRequest() { ... }` | - |
| delete_nodes_request | `on <id>.deleteNodesRequest(nodes) { ... }` | Variant nodes |
| disconnection_request | `on <id>.disconnectionRequest(fromNode, fromPort, toNode, toPort) { ... }` | Variant fromNode, int fromPort, Variant toNode, int toPort |
| duplicate_nodes_request | `on <id>.duplicateNodesRequest() { ... }` | - |
| end_node_move | `on <id>.endNodeMove() { ... }` | - |
| frame_rect_changed | `on <id>.frameRectChanged(frame, newRect) { ... }` | Object frame, Variant newRect |
| graph_elements_linked_to_frame_request | `on <id>.graphElementsLinkedToFrameRequest(elements, frame) { ... }` | Variant elements, Variant frame |
| node_deselected | `on <id>.nodeDeselected(node) { ... }` | Object node |
| node_selected | `on <id>.nodeSelected(node) { ... }` | Object node |
| paste_nodes_request | `on <id>.pasteNodesRequest() { ... }` | - |
| popup_request | `on <id>.popupRequest(atPosition) { ... }` | Vector2 atPosition |
| scroll_offset_changed | `on <id>.scrollOffsetChanged(offset) { ... }` | Vector2 offset |

## Runtime Actions

This page lists **callable methods declared by `GraphEdit`**.
Inherited actions are documented in: [Control](Control.md)

| Godot Method | SMS Call | Params | Returns |
|-|-|-|-|
| add_valid_connection_type | `<id>.addValidConnectionType(fromType, toType)` | int fromType, int toType | void |
| add_valid_left_disconnect_type | `<id>.addValidLeftDisconnectType(type)` | int type | void |
| add_valid_right_disconnect_type | `<id>.addValidRightDisconnectType(type)` | int type | void |
| arrange_nodes | `<id>.arrangeNodes()` | - | void |
| clear_connections | `<id>.clearConnections()` | - | void |
| force_connection_drag_end | `<id>.forceConnectionDragEnd()` | - | void |
| get_closest_connection_at_point | `<id>.getClosestConnectionAtPoint(point, maxDistance)` | Vector2 point, float maxDistance | Variant |
| get_connection_line | `<id>.getConnectionLine(fromNode, toNode)` | Vector2 fromNode, Vector2 toNode | Variant |
| get_connection_list | `<id>.getConnectionList()` | - | Variant |
| get_menu_hbox | `<id>.getMenuHbox()` | - | Object |
| is_right_disconnects_enabled | `<id>.isRightDisconnectsEnabled()` | - | bool |
| is_showing_arrange_button | `<id>.isShowingArrangeButton()` | - | bool |
| is_showing_grid | `<id>.isShowingGrid()` | - | bool |
| is_showing_grid_buttons | `<id>.isShowingGridButtons()` | - | bool |
| is_showing_menu | `<id>.isShowingMenu()` | - | bool |
| is_showing_minimap_button | `<id>.isShowingMinimapButton()` | - | bool |
| is_showing_zoom_buttons | `<id>.isShowingZoomButtons()` | - | bool |
| is_showing_zoom_label | `<id>.isShowingZoomLabel()` | - | bool |
| is_valid_connection_type | `<id>.isValidConnectionType(fromType, toType)` | int fromType, int toType | bool |
| remove_valid_connection_type | `<id>.removeValidConnectionType(fromType, toType)` | int fromType, int toType | void |
| remove_valid_left_disconnect_type | `<id>.removeValidLeftDisconnectType(type)` | int type | void |
| remove_valid_right_disconnect_type | `<id>.removeValidRightDisconnectType(type)` | int type | void |

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

