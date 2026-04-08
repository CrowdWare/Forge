# Window

## Inheritance

[Window](Window.md) → [Viewport](Viewport.md) → [Node](Node.md) → [Object](Object.md)

## Derived Classes

### Direct subclasses

- [AcceptDialog](AcceptDialog.md)
- [Popup](Popup.md)

## Properties

This page lists **only properties declared by `Window`**.
Inherited properties are documented in: [Viewport](Viewport.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|
| accessibility_description | accessibilityDescription | string | - |
| accessibility_name | accessibilityName | string | - |
| always_on_top | alwaysOnTop | bool | - |
| borderless | borderless | bool | - |
| content_scale_aspect | contentScaleAspect | int | - |
| content_scale_factor | contentScaleFactor | float | - |
| content_scale_mode | contentScaleMode | int | - |
| content_scale_stretch | contentScaleStretch | int | - |
| current_screen | currentScreen | int | - |
| exclude_from_capture | excludeFromCapture | bool | - |
| exclusive | exclusive | bool | - |
| extend_to_title | extendToTitle | bool | - |
| force_native | forceNative | bool | - |
| initial_position | initialPosition | int | - |
| keep_title_visible | keepTitleVisible | bool | - |
| maximize_disabled | maximizeDisabled | bool | - |
| minimize_disabled | minimizeDisabled | bool | - |
| mode | mode | int | - |
| mouse_passthrough | mousePassthrough | bool | - |
| popup_window | popupWindow | bool | - |
| popup_wm_hint | popupWmHint | bool | - |
| sharp_corners | sharpCorners | bool | - |
| theme_type_variation | themeTypeVariation | string | - |
| title | title | string | - |
| transient | transient | bool | - |
| transient_to_focused | transientToFocused | bool | - |
| transparent | transparent | bool | - |
| unfocusable | unfocusable | bool | - |
| unresizable | unresizable | bool | - |
| visible | visible | bool | - |
| wrap_controls | wrapControls | bool | - |

## Events

This page lists **only signals declared by `Window`**.
Inherited signals are documented in: [Viewport](Viewport.md)

| Godot Signal | SMS Event | Params |
|-|-|-|
| about_to_popup | `on <id>.aboutToPopup() { ... }` | - |
| close_requested | `on <id>.closeRequested() { ... }` | - |
| dpi_changed | `on <id>.dpiChanged() { ... }` | - |
| files_dropped | `on <id>.filesDropped(files) { ... }` | Variant files |
| focus_entered | `on <id>.focusEntered() { ... }` | - |
| focus_exited | `on <id>.focusExited() { ... }` | - |
| go_back_requested | `on <id>.goBackRequested() { ... }` | - |
| mouse_entered | `on <id>.mouseEntered() { ... }` | - |
| mouse_exited | `on <id>.mouseExited() { ... }` | - |
| nonclient_window_input | `on <id>.nonclientWindowInput(event) { ... }` | Object event |
| theme_changed | `on <id>.themeChanged() { ... }` | - |
| title_changed | `on <id>.titleChanged() { ... }` | - |
| titlebar_changed | `on <id>.titlebarChanged() { ... }` | - |
| visibility_changed | `on <id>.visibilityChanged() { ... }` | - |
| window_input | `on <id>.windowInput(event) { ... }` | Object event |

## Runtime Actions

This page lists **callable methods declared by `Window`**.
Inherited actions are documented in: [Viewport](Viewport.md)

| Godot Method | SMS Call | Params | Returns |
|-|-|-|-|
| begin_bulk_theme_override | `<id>.beginBulkThemeOverride()` | - | void |
| can_draw | `<id>.canDraw()` | - | bool |
| child_controls_changed | `<id>.childControlsChanged()` | - | void |
| end_bulk_theme_override | `<id>.endBulkThemeOverride()` | - | void |
| get_contents_minimum_size | `<id>.getContentsMinimumSize()` | - | Vector2 |
| get_flag | `<id>.getFlag(flag)` | int flag | bool |
| get_focused_window | `<id>.getFocusedWindow()` | - | Object |
| get_layout_direction | `<id>.getLayoutDirection()` | - | int |
| get_position_with_decorations | `<id>.getPositionWithDecorations()` | - | Variant |
| get_size_with_decorations | `<id>.getSizeWithDecorations()` | - | Variant |
| get_theme_default_base_scale | `<id>.getThemeDefaultBaseScale()` | - | float |
| get_theme_default_font | `<id>.getThemeDefaultFont()` | - | Object |
| get_theme_default_font_size | `<id>.getThemeDefaultFontSize()` | - | int |
| get_window_id | `<id>.getWindowId()` | - | int |
| grab_focus | `<id>.grabFocus()` | - | void |
| has_focus | `<id>.hasFocus()` | - | bool |
| hide | `<id>.hide()` | - | void |
| is_auto_translating | `<id>.isAutoTranslating()` | - | bool |
| is_embedded | `<id>.isEmbedded()` | - | bool |
| is_layout_rtl | `<id>.isLayoutRtl()` | - | bool |
| is_maximize_allowed | `<id>.isMaximizeAllowed()` | - | bool |
| is_using_font_oversampling | `<id>.isUsingFontOversampling()` | - | bool |
| is_wrapping_controls | `<id>.isWrappingControls()` | - | bool |
| move_to_center | `<id>.moveToCenter()` | - | void |
| move_to_foreground | `<id>.moveToForeground()` | - | void |
| popup_centered_ratio | `<id>.popupCenteredRatio(ratio)` | float ratio | void |
| request_attention | `<id>.requestAttention()` | - | void |
| reset_size | `<id>.resetSize()` | - | void |
| set_flag | `<id>.setFlag(flag, enabled)` | int flag, bool enabled | void |
| set_ime_active | `<id>.setImeActive(active)` | bool active | void |
| set_layout_direction | `<id>.setLayoutDirection(direction)` | int direction | void |
| set_unparent_when_invisible | `<id>.setUnparentWhenInvisible(unparent)` | bool unparent | void |
| set_use_font_oversampling | `<id>.setUseFontOversampling(enable)` | bool enable | void |
| show | `<id>.show()` | - | void |
| start_drag | `<id>.startDrag()` | - | void |
| start_resize | `<id>.startResize(edge)` | int edge | void |

## Actions

This page lists **only actions supported by the runtime** for `Window`.
Inherited actions are documented in: [Viewport](Viewport.md)

| Action | SMS Call | Params | Returns |
|-|-|-|-|
| onClose | `<id>.onClose(callbackName)` | string callbackName | void |
