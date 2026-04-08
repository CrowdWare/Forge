# Viewport

> Note: This is a base class included for inheritance documentation. It is **not** an SML element.

## Inheritance

[Viewport](Viewport.md) → [Node](Node.md) → [Object](Object.md)

## Derived Classes

### Direct subclasses

- [SubViewport](SubViewport.md)
- [Window](Window.md)

## Properties

This page lists **only properties declared by `Viewport`**.
Inherited properties are documented in: [Node](Node.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|
| anisotropic_filtering_level | anisotropicFilteringLevel | int | - |
| audio_listener_enable_2d | audioListenerEnable2d | bool | - |
| audio_listener_enable_3d | audioListenerEnable3d | bool | - |
| canvas_cull_mask | canvasCullMask | int | - |
| canvas_item_default_texture_filter | canvasItemDefaultTextureFilter | int | - |
| canvas_item_default_texture_repeat | canvasItemDefaultTextureRepeat | int | - |
| debug_draw | debugDraw | int | - |
| disable_3d | disable3d | bool | - |
| fsr_sharpness | fsrSharpness | float | - |
| gui_disable_input | guiDisableInput | bool | - |
| gui_drag_threshold | guiDragThreshold | int | - |
| gui_embed_subwindows | guiEmbedSubwindows | bool | - |
| gui_snap_controls_to_pixels | guiSnapControlsToPixels | bool | - |
| handle_input_locally | handleInputLocally | bool | - |
| mesh_lod_threshold | meshLodThreshold | float | - |
| msaa_2d | msaa2d | int | - |
| msaa_3d | msaa3d | int | - |
| oversampling | oversampling | bool | - |
| oversampling_override | oversamplingOverride | float | - |
| own_world_3d | ownWorld3d | bool | - |
| physics_object_picking | physicsObjectPicking | bool | - |
| physics_object_picking_first_only | physicsObjectPickingFirstOnly | bool | - |
| physics_object_picking_sort | physicsObjectPickingSort | bool | - |
| positional_shadow_atlas_16_bits | positionalShadowAtlas16Bits | bool | - |
| positional_shadow_atlas_quad_0 | positionalShadowAtlasQuad0 | int | - |
| positional_shadow_atlas_quad_1 | positionalShadowAtlasQuad1 | int | - |
| positional_shadow_atlas_quad_2 | positionalShadowAtlasQuad2 | int | - |
| positional_shadow_atlas_quad_3 | positionalShadowAtlasQuad3 | int | - |
| positional_shadow_atlas_size | positionalShadowAtlasSize | int | - |
| scaling_3d_mode | scaling3dMode | int | - |
| scaling_3d_scale | scaling3dScale | float | - |
| screen_space_aa | screenSpaceAa | int | - |
| sdf_oversize | sdfOversize | int | - |
| sdf_scale | sdfScale | int | - |
| snap_2d_transforms_to_pixel | snap2dTransformsToPixel | bool | - |
| snap_2d_vertices_to_pixel | snap2dVerticesToPixel | bool | - |
| texture_mipmap_bias | textureMipmapBias | float | - |
| transparent_bg | transparentBg | bool | - |
| use_debanding | useDebanding | bool | - |
| use_hdr_2d | useHdr2d | bool | - |
| use_occlusion_culling | useOcclusionCulling | bool | - |
| use_taa | useTaa | bool | - |
| use_xr | useXr | bool | - |
| vrs_mode | vrsMode | int | - |
| vrs_update_mode | vrsUpdateMode | int | - |

## Events

This page lists **only signals declared by `Viewport`**.
Inherited signals are documented in: [Node](Node.md)

| Godot Signal | SMS Event | Params |
|-|-|-|
| gui_focus_changed | `on <id>.guiFocusChanged(node) { ... }` | Object node |
| size_changed | `on <id>.sizeChanged() { ... }` | - |

## Runtime Actions

This page lists **callable methods declared by `Viewport`**.
Inherited actions are documented in: [Node](Node.md)

| Godot Method | SMS Call | Params | Returns |
|-|-|-|-|
| find_world_2d | `<id>.findWorld2d()` | - | Object |
| find_world_3d | `<id>.findWorld3d()` | - | Object |
| get_audio_listener_2d | `<id>.getAudioListener2d()` | - | Object |
| get_audio_listener_3d | `<id>.getAudioListener3d()` | - | Object |
| get_camera_2d | `<id>.getCamera2d()` | - | Object |
| get_camera_3d | `<id>.getCamera3d()` | - | Object |
| get_canvas_cull_mask_bit | `<id>.getCanvasCullMaskBit(layer)` | int layer | bool |
| get_default_canvas_item_texture_filter | `<id>.getDefaultCanvasItemTextureFilter()` | - | int |
| get_default_canvas_item_texture_repeat | `<id>.getDefaultCanvasItemTextureRepeat()` | - | int |
| get_drag_threshold | `<id>.getDragThreshold()` | - | int |
| get_embedded_subwindows | `<id>.getEmbeddedSubwindows()` | - | Variant |
| get_final_transform | `<id>.getFinalTransform()` | - | Variant |
| get_mouse_position | `<id>.getMousePosition()` | - | Vector2 |
| get_positional_shadow_atlas_quadrant_subdiv | `<id>.getPositionalShadowAtlasQuadrantSubdiv(quadrant)` | int quadrant | int |
| get_render_info | `<id>.getRenderInfo(type, info)` | int type, int info | int |
| get_screen_transform | `<id>.getScreenTransform()` | - | Variant |
| get_stretch_transform | `<id>.getStretchTransform()` | - | Variant |
| get_texture | `<id>.getTexture()` | - | Object |
| get_viewport_rid | `<id>.getViewportRid()` | - | Variant |
| get_visible_rect | `<id>.getVisibleRect()` | - | Variant |
| gui_cancel_drag | `<id>.guiCancelDrag()` | - | void |
| gui_get_drag_data | `<id>.guiGetDragData()` | - | void |
| gui_get_drag_description | `<id>.guiGetDragDescription()` | - | string |
| gui_get_focus_owner | `<id>.guiGetFocusOwner()` | - | Object |
| gui_get_hovered_control | `<id>.guiGetHoveredControl()` | - | Object |
| gui_is_drag_successful | `<id>.guiIsDragSuccessful()` | - | bool |
| gui_is_dragging | `<id>.guiIsDragging()` | - | bool |
| gui_release_focus | `<id>.guiReleaseFocus()` | - | void |
| gui_set_drag_description | `<id>.guiSetDragDescription(description)` | string description | void |
| has_transparent_background | `<id>.hasTransparentBackground()` | - | bool |
| is_3d_disabled | `<id>.is3dDisabled()` | - | bool |
| is_audio_listener_2d | `<id>.isAudioListener2d()` | - | bool |
| is_audio_listener_3d | `<id>.isAudioListener3d()` | - | bool |
| is_embedding_subwindows | `<id>.isEmbeddingSubwindows()` | - | bool |
| is_handling_input_locally | `<id>.isHandlingInputLocally()` | - | bool |
| is_input_disabled | `<id>.isInputDisabled()` | - | bool |
| is_input_handled | `<id>.isInputHandled()` | - | bool |
| is_snap_2d_transforms_to_pixel_enabled | `<id>.isSnap2dTransformsToPixelEnabled()` | - | bool |
| is_snap_2d_vertices_to_pixel_enabled | `<id>.isSnap2dVerticesToPixelEnabled()` | - | bool |
| is_snap_controls_to_pixels_enabled | `<id>.isSnapControlsToPixelsEnabled()` | - | bool |
| is_using_debanding | `<id>.isUsingDebanding()` | - | bool |
| is_using_hdr_2d | `<id>.isUsingHdr2d()` | - | bool |
| is_using_occlusion_culling | `<id>.isUsingOcclusionCulling()` | - | bool |
| is_using_oversampling | `<id>.isUsingOversampling()` | - | bool |
| is_using_own_world_3d | `<id>.isUsingOwnWorld3d()` | - | bool |
| is_using_taa | `<id>.isUsingTaa()` | - | bool |
| is_using_xr | `<id>.isUsingXr()` | - | bool |
| notify_mouse_entered | `<id>.notifyMouseEntered()` | - | void |
| notify_mouse_exited | `<id>.notifyMouseExited()` | - | void |
| push_text_input | `<id>.pushTextInput(text)` | string text | void |
| set_as_audio_listener_2d | `<id>.setAsAudioListener2d(enable)` | bool enable | void |
| set_as_audio_listener_3d | `<id>.setAsAudioListener3d(enable)` | bool enable | void |
| set_canvas_cull_mask_bit | `<id>.setCanvasCullMaskBit(layer, enable)` | int layer, bool enable | void |
| set_default_canvas_item_texture_filter | `<id>.setDefaultCanvasItemTextureFilter(mode)` | int mode | void |
| set_default_canvas_item_texture_repeat | `<id>.setDefaultCanvasItemTextureRepeat(mode)` | int mode | void |
| set_disable_input | `<id>.setDisableInput(disable)` | bool disable | void |
| set_drag_threshold | `<id>.setDragThreshold(threshold)` | int threshold | void |
| set_embedding_subwindows | `<id>.setEmbeddingSubwindows(enable)` | bool enable | void |
| set_input_as_handled | `<id>.setInputAsHandled()` | - | void |
| set_positional_shadow_atlas_quadrant_subdiv | `<id>.setPositionalShadowAtlasQuadrantSubdiv(quadrant, subdiv)` | int quadrant, int subdiv | void |
| set_snap_controls_to_pixels | `<id>.setSnapControlsToPixels(enabled)` | bool enabled | void |
| set_transparent_background | `<id>.setTransparentBackground(enable)` | bool enable | void |
| set_use_oversampling | `<id>.setUseOversampling(enable)` | bool enable | void |
| set_use_own_world_3d | `<id>.setUseOwnWorld3d(enable)` | bool enable | void |
| update_mouse_cursor_state | `<id>.updateMouseCursorState()` | - | void |
| warp_mouse | `<id>.warpMouse(position)` | Vector2 position | void |
