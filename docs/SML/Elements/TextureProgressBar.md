# TextureProgressBar

## Inheritance

[TextureProgressBar](TextureProgressBar.md) → [Range](Range.md) → [Control](Control.md) → [CanvasItem](CanvasItem.md) → [Node](Node.md) → [Object](Object.md)

## Properties

This page lists **only properties declared by `TextureProgressBar`**.
Inherited properties are documented in: [Range](Range.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|
| fill_mode | fillMode | int | - |
| nine_patch_stretch | ninePatchStretch | bool | - |
| radial_center_offset | radialCenterOffset | Vector2 | - |
| radial_fill_degrees | radialFillDegrees | float | - |
| radial_initial_angle | radialInitialAngle | float | - |
| stretch_margin_bottom | stretchMarginBottom | int | - |
| stretch_margin_left | stretchMarginLeft | int | - |
| stretch_margin_right | stretchMarginRight | int | - |
| stretch_margin_top | stretchMarginTop | int | - |
| texture_progress_offset | textureProgressOffset | Vector2 | - |
| tint_over | tintOver | Color | - |
| tint_progress | tintProgress | Color | - |
| tint_under | tintUnder | Color | - |

## Events

This page lists **only signals declared by `TextureProgressBar`**.
Inherited signals are documented in: [Range](Range.md)

| Godot Signal | SMS Event | Params |
|-|-|-|

## Runtime Actions

This page lists **callable methods declared by `TextureProgressBar`**.
Inherited actions are documented in: [Range](Range.md)

| Godot Method | SMS Call | Params | Returns |
|-|-|-|-|
| get_fill_degrees | `<id>.getFillDegrees()` | - | float |
| get_over_texture | `<id>.getOverTexture()` | - | Object |
| get_progress_texture | `<id>.getProgressTexture()` | - | Object |
| get_stretch_margin | `<id>.getStretchMargin(margin)` | int margin | int |
| get_under_texture | `<id>.getUnderTexture()` | - | Object |
| set_fill_degrees | `<id>.setFillDegrees(mode)` | float mode | void |
| set_stretch_margin | `<id>.setStretchMargin(margin, value)` | int margin, int value | void |

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

