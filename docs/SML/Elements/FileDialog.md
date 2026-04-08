# FileDialog

## Inheritance

[FileDialog](FileDialog.md) → [ConfirmationDialog](ConfirmationDialog.md) → [AcceptDialog](AcceptDialog.md) → [Window](Window.md) → [Viewport](Viewport.md) → [Node](Node.md) → [Object](Object.md)

## Derived Classes

### Direct subclasses

- [EditorFileDialog](EditorFileDialog.md)

## Properties

This page lists **only properties declared by `FileDialog`**.
Inherited properties are documented in: [ConfirmationDialog](ConfirmationDialog.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|
| access | access | int | - |
| deleting_enabled | deletingEnabled | bool | - |
| display_mode | displayMode | int | - |
| favorites_enabled | favoritesEnabled | bool | - |
| file_filter_toggle_enabled | fileFilterToggleEnabled | bool | - |
| file_mode | fileMode | int | - |
| file_sort_options_enabled | fileSortOptionsEnabled | bool | - |
| filename_filter | filenameFilter | string | - |
| folder_creation_enabled | folderCreationEnabled | bool | - |
| hidden_files_toggle_enabled | hiddenFilesToggleEnabled | bool | - |
| layout_toggle_enabled | layoutToggleEnabled | bool | - |
| mode_overrides_title | modeOverridesTitle | bool | - |
| option_count | optionCount | int | - |
| overwrite_warning_enabled | overwriteWarningEnabled | bool | - |
| recent_list_enabled | recentListEnabled | bool | - |
| root_subfolder | rootSubfolder | string | - |
| show_hidden_files | showHiddenFiles | bool | - |
| use_native_dialog | useNativeDialog | bool | - |

## Events

This page lists **only signals declared by `FileDialog`**.
Inherited signals are documented in: [ConfirmationDialog](ConfirmationDialog.md)

| Godot Signal | SMS Event | Params |
|-|-|-|
| dir_selected | `on <id>.dirSelected(dir) { ... }` | string dir |
| file_selected | `on <id>.fileSelected(path) { ... }` | string path |
| filename_filter_changed | `on <id>.filenameFilterChanged(filter) { ... }` | string filter |
| files_selected | `on <id>.filesSelected(paths) { ... }` | Variant paths |

## Runtime Actions

This page lists **callable methods declared by `FileDialog`**.
Inherited actions are documented in: [ConfirmationDialog](ConfirmationDialog.md)

| Godot Method | SMS Call | Params | Returns |
|-|-|-|-|
| add_filter | `<id>.addFilter(filter, description, mimeType)` | string filter, string description, string mimeType | void |
| clear_filename_filter | `<id>.clearFilenameFilter()` | - | void |
| clear_filters | `<id>.clearFilters()` | - | void |
| deselect_all | `<id>.deselectAll()` | - | void |
| get_favorite_list | `<id>.getFavoriteList()` | - | Variant |
| get_line_edit | `<id>.getLineEdit()` | - | Object |
| get_option_default | `<id>.getOptionDefault(option)` | int option | int |
| get_option_name | `<id>.getOptionName(option)` | int option | string |
| get_option_values | `<id>.getOptionValues(option)` | int option | Variant |
| get_recent_list | `<id>.getRecentList()` | - | Variant |
| get_selected_options | `<id>.getSelectedOptions()` | - | Variant |
| get_vbox | `<id>.getVbox()` | - | Object |
| invalidate | `<id>.invalidate()` | - | void |
| is_customization_flag_enabled | `<id>.isCustomizationFlagEnabled(flag)` | int flag | bool |
| is_mode_overriding_title | `<id>.isModeOverridingTitle()` | - | bool |
| is_showing_hidden_files | `<id>.isShowingHiddenFiles()` | - | bool |
| popup_file_dialog | `<id>.popupFileDialog()` | - | void |
| set_customization_flag_enabled | `<id>.setCustomizationFlagEnabled(flag, enabled)` | int flag, bool enabled | void |
| set_option_default | `<id>.setOptionDefault(option, defaultValueIndex)` | int option, int defaultValueIndex | void |
| set_option_name | `<id>.setOptionName(option, name)` | int option, string name | void |
