# ColorPicker

## Inheritance

[ColorPicker](ColorPicker.md) → [VBoxContainer](VBoxContainer.md) → [BoxContainer](BoxContainer.md) → [Container](Container.md) → [Control](Control.md) → [CanvasItem](CanvasItem.md) → [Node](Node.md) → [Object](Object.md)

## Properties

This page lists **only properties declared by `ColorPicker`**.
Inherited properties are documented in: [VBoxContainer](VBoxContainer.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|
| can_add_swatches | canAddSwatches | bool | - |
| color | color | Color | - |
| color_mode | colorMode | int | - |
| color_modes_visible | colorModesVisible | bool | - |
| deferred_mode | deferredMode | bool | - |
| edit_alpha | editAlpha | bool | - |
| edit_intensity | editIntensity | bool | - |
| hex_visible | hexVisible | bool | - |
| picker_shape | pickerShape | int | - |
| presets_visible | presetsVisible | bool | - |
| sampler_visible | samplerVisible | bool | - |
| sliders_visible | slidersVisible | bool | - |

## Events

This page lists **only signals declared by `ColorPicker`**.
Inherited signals are documented in: [VBoxContainer](VBoxContainer.md)

| Godot Signal | SMS Event | Params |
|-|-|-|
| color_changed | `on <id>.colorChanged(color) { ... }` | Color color |
| preset_added | `on <id>.presetAdded(color) { ... }` | Color color |
| preset_removed | `on <id>.presetRemoved(color) { ... }` | Color color |

## Runtime Actions

This page lists **callable methods declared by `ColorPicker`**.
Inherited actions are documented in: [VBoxContainer](VBoxContainer.md)

| Godot Method | SMS Call | Params | Returns |
|-|-|-|-|
| add_preset | `<id>.addPreset(color)` | Color color | void |
| add_recent_preset | `<id>.addRecentPreset(color)` | Color color | void |
| are_modes_visible | `<id>.areModesVisible()` | - | bool |
| are_presets_visible | `<id>.arePresetsVisible()` | - | bool |
| are_sliders_visible | `<id>.areSlidersVisible()` | - | bool |
| are_swatches_enabled | `<id>.areSwatchesEnabled()` | - | bool |
| erase_preset | `<id>.erasePreset(color)` | Color color | void |
| erase_recent_preset | `<id>.eraseRecentPreset(color)` | Color color | void |
| get_pick_color | `<id>.getPickColor()` | - | Color |
| get_presets | `<id>.getPresets()` | - | Variant |
| get_recent_presets | `<id>.getRecentPresets()` | - | Variant |
| is_editing_alpha | `<id>.isEditingAlpha()` | - | bool |
| is_editing_intensity | `<id>.isEditingIntensity()` | - | bool |
| set_modes_visible | `<id>.setModesVisible(visible)` | bool visible | void |
| set_pick_color | `<id>.setPickColor(color)` | Color color | void |

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

