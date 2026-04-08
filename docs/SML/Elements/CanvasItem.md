# CanvasItem

> Note: This is a base class included for inheritance documentation. It is **not** an SML element.

## Inheritance

[CanvasItem](CanvasItem.md) → [Node](Node.md) → [Object](Object.md)

## Derived Classes

Classes listed below inherit from `CanvasItem`.

### Direct subclasses

- [Control](Control.md)
- [Node2D](Node2D.md)

## Properties

This page lists **only properties declared by `CanvasItem`**.
Inherited properties are documented in: [Node](Node.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|
| clip_children | clipChildren | int | - |
| light_mask | lightMask | int | - |
| modulate | modulate | Color | - |
| self_modulate | selfModulate | Color | - |
| show_behind_parent | showBehindParent | bool | - |
| texture_filter | textureFilter | int | - |
| texture_repeat | textureRepeat | int | - |
| top_level | topLevel | bool | - |
| use_parent_material | useParentMaterial | bool | - |
| visibility_layer | visibilityLayer | int | - |
| visible | visible | bool | - |
| y_sort_enabled | ySortEnabled | bool | - |
| z_as_relative | zAsRelative | bool | - |
| z_index | zIndex | int | - |

## Events

This page lists **only signals declared by `CanvasItem`**.
Inherited signals are documented in: [Node](Node.md)

| Godot Signal | SMS Event | Params |
|-|-|-|
| draw | `on <id>.draw() { ... }` | - |
| hidden | `on <id>.hidden() { ... }` | - |
| item_rect_changed | `on <id>.itemRectChanged() { ... }` | - |
| visibility_changed | `on <id>.visibilityChanged() { ... }` | - |

## Runtime Actions

This page lists **callable methods declared by `CanvasItem`**.
Inherited actions are documented in: [Node](Node.md)

| Godot Method | SMS Call | Params | Returns |
|-|-|-|-|
| draw_animation_slice | `<id>.drawAnimationSlice(animationLength, sliceBegin, sliceEnd, offset)` | float animationLength, float sliceBegin, float sliceEnd, float offset | void |
| draw_arc | `<id>.drawArc(center, radius, startAngle, endAngle, pointCount, color, width, antialiased)` | Vector2 center, float radius, float startAngle, float endAngle, int pointCount, Color color, float width, bool antialiased | void |
| draw_circle | `<id>.drawCircle(position, radius, color, filled, width, antialiased)` | Vector2 position, float radius, Color color, bool filled, float width, bool antialiased | void |
| draw_dashed_line | `<id>.drawDashedLine(from, to, color, width, dash, aligned, antialiased)` | Vector2 from, Vector2 to, Color color, float width, float dash, bool aligned, bool antialiased | void |
| draw_ellipse | `<id>.drawEllipse(position, major, minor, color, filled, width, antialiased)` | Vector2 position, float major, float minor, Color color, bool filled, float width, bool antialiased | void |
| draw_ellipse_arc | `<id>.drawEllipseArc(center, major, minor, startAngle, endAngle, pointCount, color, width, antialiased)` | Vector2 center, float major, float minor, float startAngle, float endAngle, int pointCount, Color color, float width, bool antialiased | void |
| draw_end_animation | `<id>.drawEndAnimation()` | - | void |
| draw_line | `<id>.drawLine(from, to, color, width, antialiased)` | Vector2 from, Vector2 to, Color color, float width, bool antialiased | void |
| draw_set_transform | `<id>.drawSetTransform(position, rotation, scale)` | Vector2 position, float rotation, Vector2 scale | void |
| force_update_transform | `<id>.forceUpdateTransform()` | - | void |
| get_canvas | `<id>.getCanvas()` | - | Variant |
| get_canvas_item | `<id>.getCanvasItem()` | - | Variant |
| get_canvas_layer_node | `<id>.getCanvasLayerNode()` | - | Object |
| get_canvas_transform | `<id>.getCanvasTransform()` | - | Variant |
| get_clip_children_mode | `<id>.getClipChildrenMode()` | - | int |
| get_global_mouse_position | `<id>.getGlobalMousePosition()` | - | Vector2 |
| get_global_transform | `<id>.getGlobalTransform()` | - | Variant |
| get_global_transform_with_canvas | `<id>.getGlobalTransformWithCanvas()` | - | Variant |
| get_local_mouse_position | `<id>.getLocalMousePosition()` | - | Vector2 |
| get_screen_transform | `<id>.getScreenTransform()` | - | Variant |
| get_transform | `<id>.getTransform()` | - | Variant |
| get_viewport_rect | `<id>.getViewportRect()` | - | Variant |
| get_viewport_transform | `<id>.getViewportTransform()` | - | Variant |
| get_visibility_layer_bit | `<id>.getVisibilityLayerBit(layer)` | int layer | bool |
| get_world_2d | `<id>.getWorld2d()` | - | Object |
| hide | `<id>.hide()` | - | void |
| is_draw_behind_parent_enabled | `<id>.isDrawBehindParentEnabled()` | - | bool |
| is_local_transform_notification_enabled | `<id>.isLocalTransformNotificationEnabled()` | - | bool |
| is_set_as_top_level | `<id>.isSetAsTopLevel()` | - | bool |
| is_transform_notification_enabled | `<id>.isTransformNotificationEnabled()` | - | bool |
| is_visible_in_tree | `<id>.isVisibleInTree()` | - | bool |
| is_z_relative | `<id>.isZRelative()` | - | bool |
| make_canvas_position_local | `<id>.makeCanvasPositionLocal(viewportPoint)` | Vector2 viewportPoint | Vector2 |
| move_to_front | `<id>.moveToFront()` | - | void |
| queue_redraw | `<id>.queueRedraw()` | - | void |
| set_as_top_level | `<id>.setAsTopLevel(enable)` | bool enable | void |
| set_clip_children_mode | `<id>.setClipChildrenMode(mode)` | int mode | void |
| set_draw_behind_parent | `<id>.setDrawBehindParent(enable)` | bool enable | void |
| set_notify_local_transform | `<id>.setNotifyLocalTransform(enable)` | bool enable | void |
| set_notify_transform | `<id>.setNotifyTransform(enable)` | bool enable | void |
| set_visibility_layer_bit | `<id>.setVisibilityLayerBit(layer, enabled)` | int layer, bool enabled | void |
| show | `<id>.show()` | - | void |
