# ProgressBar

## Inheritance

[ProgressBar](ProgressBar.md) → [Range](Range.md) → [Control](Control.md) → [CanvasItem](CanvasItem.md) → [Node](Node.md) → [Object](Object.md)

## Properties

This page lists **only properties declared by `ProgressBar`**.
Inherited properties are documented in: [Range](Range.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|
| editor_preview_indeterminate | editorPreviewIndeterminate | bool | - |
| fill_mode | fillMode | int | - |
| indeterminate | indeterminate | bool | - |
| show_percentage | showPercentage | bool | - |

## Events

This page lists **only signals declared by `ProgressBar`**.
Inherited signals are documented in: [Range](Range.md)

| Godot Signal | SMS Event | Params |
|-|-|-|

## Runtime Actions

This page lists **callable methods declared by `ProgressBar`**.
Inherited actions are documented in: [Range](Range.md)

| Godot Method | SMS Call | Params | Returns |
|-|-|-|-|
| is_editor_preview_indeterminate_enabled | `<id>.isEditorPreviewIndeterminateEnabled()` | - | bool |
| is_percentage_shown | `<id>.isPercentageShown()` | - | bool |

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

