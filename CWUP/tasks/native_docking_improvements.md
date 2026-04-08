# Native Docking System Improvements

## Goal
Complete the ForgeDockingHostControl / ForgeDockingContainerControl C++ implementation
with feature parity to the C# DockingHostControl.

## Context
The kebab menu dialog, floating windows, and auto-dock-slot creation are done.
The following features are still missing vs. the C# reference implementation.

## Tasks

### 1 - Panel collapse when empty
When a `ForgeDockingContainerControl` loses its last tab (tab count → 0), it must set
`visible = false`. The host calls `queue_sort()` so the layout reflows without the
collapsed column.

Reference (C#): `DockingContainerControl` calls `Host.QueueSort()` after a tab is
removed when `GetTabCount() == 0`.

### 2 - AlwaysOnTop for floating windows
When a panel is made floating (`_make_floating()`), set `FLAG_ALWAYS_ON_TOP` on the
created `Window`:

```cpp
win->set_flag(Window::FLAG_ALWAYS_ON_TOP, true);
```

### 3 - Far column visibility on move
When a panel is moved to `farLeft`, `farLeftBottom`, `farRight`, or `farRightBottom`
and that column currently contains only invisible/empty containers, make the target
container visible.

Reference (C#): `DockingHostControl.DockPanel()` calls `container.Visible = true` after
adding the tab, then calls `QueueSort()`.

### 4 - *Bottom split layout
When a panel is moved to a `*Bottom` slot (`leftBottom`, `rightBottom`, `farLeftBottom`,
`farRightBottom`), the column must be split top/bottom:

- Top and bottom containers share the host height.
- Priority: `fixed_height_` > `height_percent_` (0–100) > automatic 50/50 default.
- Helper needed: `resolve_top_split_height(top, bottom, available_h)` → int.

Reference (C#): `DockColumn.LayoutColumn()` + `ResolveTopSplitHeight()` in
`DockingHostControl.cs`.

### 5 - Horizontal resize handles (column gap)
Add a `ColorRect` resize handle in each interior column gap:

- Cursor: `CURSOR_HSIZE`
- On drag: update `fixed_width_` of the left neighbour; clamp to `min_fixed_width_`.
- Colour: transparent when idle, semi-opaque (e.g. `(0,0,0,0.35)`) during drag.
- ZIndex: 900.

Reference (C#): `DockResizeHandleControl` with `LeftNeighbor` / `RightNeighbor` in
`DockingHostControl.cs`.

### 6 - Vertical resize handles (top/bottom split gap)
Add a `ColorRect` resize handle in the gap between top and bottom panels of a split
column:

- Cursor: `CURSOR_VSIZE`
- On drag: update `height_percent_` of the top container; clamp to `min_fixed_height_`.
- Same colour convention as horizontal handle.
- ZIndex: 900.

Reference (C#): `DockingHostControl.Vertical.cs` - `AddVerticalResizeHandle()` /
`OnVerticalHandleInput()`.

## Acceptance Criteria
- Dragging a panel to an empty dock slot makes that slot visible.
- Removing the last tab from a dock container hides it and reflows the layout.
- Floating windows appear always-on-top.
- Moving a panel to `*Bottom` visually splits the column.
- Dragging the gap between columns resizes the left column width.
- Dragging the gap between top/bottom panels adjusts the split ratio.
