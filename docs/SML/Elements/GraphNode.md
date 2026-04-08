# GraphNode

## Inheritance

[GraphNode](GraphNode.md) → [GraphElement](GraphElement.md) → [Container](Container.md) → [Control](Control.md) → [CanvasItem](CanvasItem.md) → [Node](Node.md) → [Object](Object.md)

## Properties

This page lists **only properties declared by `GraphNode`**.
Inherited properties are documented in: [GraphElement](GraphElement.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|
| ignore_invalid_connection_type | ignoreInvalidConnectionType | bool | - |
| slots_focus_mode | slotsFocusMode | int | - |
| title | title | string | - |

## Events

This page lists **only signals declared by `GraphNode`**.
Inherited signals are documented in: [GraphElement](GraphElement.md)

| Godot Signal | SMS Event | Params |
|-|-|-|
| slot_sizes_changed | `on <id>.slotSizesChanged() { ... }` | - |
| slot_updated | `on <id>.slotUpdated(slotIndex) { ... }` | int slotIndex |

## Runtime Actions

This page lists **callable methods declared by `GraphNode`**.
Inherited actions are documented in: [GraphElement](GraphElement.md)

| Godot Method | SMS Call | Params | Returns |
|-|-|-|-|
| clear_all_slots | `<id>.clearAllSlots()` | - | void |
| clear_slot | `<id>.clearSlot(slotIndex)` | int slotIndex | void |
| get_input_port_color | `<id>.getInputPortColor(portIdx)` | int portIdx | Color |
| get_input_port_count | `<id>.getInputPortCount()` | - | int |
| get_input_port_position | `<id>.getInputPortPosition(portIdx)` | int portIdx | Vector2 |
| get_input_port_slot | `<id>.getInputPortSlot(portIdx)` | int portIdx | int |
| get_input_port_type | `<id>.getInputPortType(portIdx)` | int portIdx | int |
| get_output_port_color | `<id>.getOutputPortColor(portIdx)` | int portIdx | Color |
| get_output_port_count | `<id>.getOutputPortCount()` | - | int |
| get_output_port_position | `<id>.getOutputPortPosition(portIdx)` | int portIdx | Vector2 |
| get_output_port_slot | `<id>.getOutputPortSlot(portIdx)` | int portIdx | int |
| get_output_port_type | `<id>.getOutputPortType(portIdx)` | int portIdx | int |
| get_slot_color_left | `<id>.getSlotColorLeft(slotIndex)` | int slotIndex | Color |
| get_slot_color_right | `<id>.getSlotColorRight(slotIndex)` | int slotIndex | Color |
| get_slot_custom_icon_left | `<id>.getSlotCustomIconLeft(slotIndex)` | int slotIndex | Object |
| get_slot_custom_icon_right | `<id>.getSlotCustomIconRight(slotIndex)` | int slotIndex | Object |
| get_slot_metadata_left | `<id>.getSlotMetadataLeft(slotIndex)` | int slotIndex | void |
| get_slot_metadata_right | `<id>.getSlotMetadataRight(slotIndex)` | int slotIndex | void |
| get_slot_type_left | `<id>.getSlotTypeLeft(slotIndex)` | int slotIndex | int |
| get_slot_type_right | `<id>.getSlotTypeRight(slotIndex)` | int slotIndex | int |
| get_titlebar_hbox | `<id>.getTitlebarHbox()` | - | Object |
| is_ignoring_valid_connection_type | `<id>.isIgnoringValidConnectionType()` | - | bool |
| is_slot_draw_stylebox | `<id>.isSlotDrawStylebox(slotIndex)` | int slotIndex | bool |
| is_slot_enabled_left | `<id>.isSlotEnabledLeft(slotIndex)` | int slotIndex | bool |
| is_slot_enabled_right | `<id>.isSlotEnabledRight(slotIndex)` | int slotIndex | bool |
| set_slot_color_left | `<id>.setSlotColorLeft(slotIndex, color)` | int slotIndex, Color color | void |
| set_slot_color_right | `<id>.setSlotColorRight(slotIndex, color)` | int slotIndex, Color color | void |
| set_slot_draw_stylebox | `<id>.setSlotDrawStylebox(slotIndex, enable)` | int slotIndex, bool enable | void |
| set_slot_enabled_left | `<id>.setSlotEnabledLeft(slotIndex, enable)` | int slotIndex, bool enable | void |
| set_slot_enabled_right | `<id>.setSlotEnabledRight(slotIndex, enable)` | int slotIndex, bool enable | void |
| set_slot_type_left | `<id>.setSlotTypeLeft(slotIndex, type)` | int slotIndex, int type | void |
| set_slot_type_right | `<id>.setSlotTypeRight(slotIndex, type)` | int slotIndex, int type | void |

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

