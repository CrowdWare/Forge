# Native Post-Build Pass

## Goal
Implement the post-build property passes in `ForgeRunner.Native` that cannot be
applied inline during node construction (because they depend on parent/sibling
state or require multi-property coordination).

## Context
The C# `NodePropertyMapper` and `SmlUiBuilder` split property application into
an inline pass (during node creation) and several post-build passes. These are
not yet implemented in `forge_ui_builder.cpp`.

## Passes to Implement

### 1 - fontFace + fontWeight
In C#: both properties are buffered in node meta. After the full tree is built,
a post pass reads `(fontFace, fontWeight)` pairs, loads the correct
`FontFile` / `SystemFont` resource, and sets it via theme override.

Implementation in C++:
- During `apply_props()`: store `font_face` and `font_weight` in a deferred list.
- After `build()` completes: resolve font resource and call
  `ctrl->add_theme_font_override("font", ...)`.

### 2 - offsetTop / offsetBottom / offsetLeft / offsetRight
Margin offsets applied to a child after the parent layout has been established.
These are applied as `Control::set_offset(SIDE_*, value)` in the post-build pass,
not during node construction (to avoid layout-loop with anchors).

In C++: collect `(ctrl, side, value)` tuples during `apply_props()`, apply all
after the tree is fully built.

### 3 - Elevation expansion
Properties from `Elevations.name` block (in theme.sml) must be injected into the
node before it is built. This is a **pre-build** pass (see also
`tasks/native_theme_localization.md`). Covered here for cross-reference.

### 4 - Attached properties (context properties)
E.g. `leftDock.title: "Project"` on a child of `ForgeDockingContainerControl`.
These are already partially handled in `forge_runner_extension.cpp` for docking.
The general mechanism needs to cover all attached-property specs from
`tools/specs/context_properties.gd`.

Current state: tab titles are set via the `ForgeDockingContainerControl`
`_notification(NOTIFICATION_CHILD_ORDER_CHANGED)` by reading child meta.
Verify this covers all cases; add any missing attached properties.

### 5 - shrinkH / shrinkV → SIZE_FLAGS
`shrinkH: true` maps to `SIZE_SHRINK_CENTER` on horizontal size flags.
This can be done inline but needs to cooperate with `expand: true` - both flags
share the same bitmask. Ensure they are combined correctly, not overwriting each other.

## Acceptance Criteria
- `fontFace: "JetBrainsMono"` + `fontWeight: bold` loads the correct font variant.
- `offsetTop: 8` on a child shifts it 8px down without breaking anchor layout.
- `shrinkH: true` + `expand: true` correctly produce `SIZE_EXPAND | SIZE_SHRINK_CENTER`.
- `leftDock.title: "Project"` shows the correct tab label.

## Reference
- C#: `ForgeRunner/Runtime/UI/NodePropertyMapper.cs`
- C#: `ForgeRunner/Runtime/UI/SmlUiBuilder.cs` (post-build pass loop)
- C#: `ForgeRunner/Generated/SchemaContextProperties.cs`
