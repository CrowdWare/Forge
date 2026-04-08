# NinePatchRect

## Inheritance

[NinePatchRect](NinePatchRect.md) → [Control](Control.md) → [CanvasItem](CanvasItem.md) → [Node](Node.md) → [Object](Object.md)

## Properties

This page lists **only properties declared by `NinePatchRect`**.
Inherited properties are documented in: [Control](Control.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|
| axis_stretch_horizontal | axisStretchHorizontal | int | - |
| axis_stretch_vertical | axisStretchVertical | int | - |
| draw_center | drawCenter | bool | - |
| patch_margin_bottom | patchMarginBottom | int | - |
| patch_margin_left | patchMarginLeft | int | - |
| patch_margin_right | patchMarginRight | int | - |
| patch_margin_top | patchMarginTop | int | - |

## Events

This page lists **only signals declared by `NinePatchRect`**.
Inherited signals are documented in: [Control](Control.md)

| Godot Signal | SMS Event | Params |
|-|-|-|
| texture_changed | `on <id>.textureChanged() { ... }` | - |

## Runtime Actions

This page lists **callable methods declared by `NinePatchRect`**.
Inherited actions are documented in: [Control](Control.md)

| Godot Method | SMS Call | Params | Returns |
|-|-|-|-|
| get_h_axis_stretch_mode | `<id>.getHAxisStretchMode()` | - | int |
| get_patch_margin | `<id>.getPatchMargin(margin)` | int margin | int |
| get_v_axis_stretch_mode | `<id>.getVAxisStretchMode()` | - | int |
| is_draw_center_enabled | `<id>.isDrawCenterEnabled()` | - | bool |
| set_h_axis_stretch_mode | `<id>.setHAxisStretchMode(mode)` | int mode | void |
| set_patch_margin | `<id>.setPatchMargin(margin, value)` | int margin, int value | void |
| set_v_axis_stretch_mode | `<id>.setVAxisStretchMode(mode)` | int mode | void |

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

