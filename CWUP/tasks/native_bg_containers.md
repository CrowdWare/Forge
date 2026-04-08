# Native Background Containers (bgColor / shadows / borders)

## Goal
Port `BgContainers.cs` to C++ GDExtension so that `bgColor`, `borderColor`,
`borderWidth`, `borderRadius`, `shadowColor`, `shadowSize`, `shadowOffsetX/Y`,
and `highlightColor` work correctly in `ForgeRunner.Native`.

## Context
The C# implementation uses custom `VBoxContainer`, `HBoxContainer`, and
`PanelContainer` subclasses that override `_Draw()` to paint StyleBoxFlat
backgrounds. In C++ these must be registered GDExtension classes.

In `forge_ui_builder.cpp` `apply_props()` currently sets some of these via
`add_theme_stylebox_override("panel", ...)` on the existing node type, which only
works for `Panel` and `PanelContainer`. For `VBoxContainer` and `HBoxContainer`
there is no built-in panel slot - a custom _draw is required.

## Classes to Add

### `ForgeBgVBoxContainer : VBoxContainer`
- Stores `StyleBoxFlat bg_style_`.
- Overrides `_draw()`: `draw_style_box(bg_style_, Rect2(Vector2(), get_size()))`.
- Properties (set via `set_meta()` + post-build pass):
  `bg_color`, `border_color`, `border_width`, `border_radius`,
  `shadow_color`, `shadow_size`, `shadow_offset_x/y`.

### `ForgeBgHBoxContainer : HBoxContainer`
Same as above.

### `ForgeBgPanel : Panel`
Already partially works via StyleBoxFlat override. Add shadow support.

### `ForgeBgPanelContainer : PanelContainer`
Same as ForgeBgPanel.

## Post-Build Pass
After `build_node()` returns for a node with bg-style properties, call
`apply_bg_style(ctrl, node)` which:
1. Detects if bg properties are present.
2. Replaces the plain `VBoxContainer` / `HBoxContainer` with the Bg variant.
   (Or: always create Bg variants when node type is VBox/HBox/Panel/PanelContainer.)
3. Builds and applies `StyleBoxFlat` with border + shadow settings.

Shadow approximation: Godot 4 `StyleBoxFlat` has `shadow_color`, `shadow_size`,
`shadow_offset`.

## highlightColor
Used for hover/selection indication. Store as `highlight_color_` on a
`ForgeBgPanel` subclass; override `_input()` or connect `mouse_entered` /
`mouse_exited` to toggle.

## Acceptance Criteria
- `bgColor: "#1a1a1a"` on VBoxContainer renders a filled background.
- `borderRadius: 8` produces rounded corners via StyleBoxFlat.
- `shadowSize: 6` / `shadowColor: "#0008"` adds a visible drop shadow.
- `highlightColor: "#ffffff20"` shows tint on hover.

## Reference
- C#: `ForgeRunner/Runtime/UI/BgContainers.cs`
- C#: `ForgeRunner/Runtime/UI/NodePropertyMapper.cs` (bgColor, shadow setters)
