# FileSystemDock

## Inheritance

[FileSystemDock](FileSystemDock.md) → [EditorDock](EditorDock.md) → [MarginContainer](MarginContainer.md) → [Container](Container.md) → [Control](Control.md) → [CanvasItem](CanvasItem.md) → [Node](Node.md) → [Object](Object.md)

## Properties

This page lists **only properties declared by `FileSystemDock`**.
Inherited properties are documented in: [EditorDock](EditorDock.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|

## Events

This page lists **only signals declared by `FileSystemDock`**.
Inherited signals are documented in: [EditorDock](EditorDock.md)

| Godot Signal | SMS Event | Params |
|-|-|-|
| display_mode_changed | `on <id>.displayModeChanged() { ... }` | - |
| file_removed | `on <id>.fileRemoved(file) { ... }` | string file |
| files_moved | `on <id>.filesMoved(oldFile, newFile) { ... }` | string oldFile, string newFile |
| folder_color_changed | `on <id>.folderColorChanged() { ... }` | - |
| folder_moved | `on <id>.folderMoved(oldFolder, newFolder) { ... }` | string oldFolder, string newFolder |
| folder_removed | `on <id>.folderRemoved(folder) { ... }` | string folder |
| inherit | `on <id>.inherit(file) { ... }` | string file |
| instantiate | `on <id>.instantiate(files) { ... }` | Variant files |
| resource_removed | `on <id>.resourceRemoved(resource) { ... }` | Object resource |
| selection_changed | `on <id>.selectionChanged() { ... }` | - |

## Runtime Actions

This page lists **callable methods declared by `FileSystemDock`**.
Inherited actions are documented in: [EditorDock](EditorDock.md)

| Godot Method | SMS Call | Params | Returns |
|-|-|-|-|
| navigate_to_path | `<id>.navigateToPath(path)` | string path | void |

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

