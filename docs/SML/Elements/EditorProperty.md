# EditorProperty

## Inheritance

[EditorProperty](EditorProperty.md) → [Container](Container.md) → [Control](Control.md) → [CanvasItem](CanvasItem.md) → [Node](Node.md) → [Object](Object.md)

## Properties

This page lists **only properties declared by `EditorProperty`**.
Inherited properties are documented in: [Container](Container.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|
| checkable | checkable | bool | - |
| checked | checked | bool | - |
| deletable | deletable | bool | - |
| draw_background | drawBackground | bool | - |
| draw_label | drawLabel | bool | - |
| draw_warning | drawWarning | bool | - |
| keying | keying | bool | - |
| label | label | string | - |
| name_split_ratio | nameSplitRatio | float | - |
| read_only | readOnly | bool | - |
| selectable | selectable | bool | - |
| use_folding | useFolding | bool | - |

## Events

This page lists **only signals declared by `EditorProperty`**.
Inherited signals are documented in: [Container](Container.md)

| Godot Signal | SMS Event | Params |
|-|-|-|
| multiple_properties_changed | `on <id>.multiplePropertiesChanged(properties, value) { ... }` | Variant properties, Variant value |
| object_id_selected | `on <id>.objectIdSelected(property, id) { ... }` | Variant property, int id |
| property_can_revert_changed | `on <id>.propertyCanRevertChanged(property, canRevert) { ... }` | Variant property, bool canRevert |
| property_changed | `on <id>.propertyChanged(property, value, field, changing) { ... }` | Variant property, Variant value, Variant field, bool changing |
| property_checked | `on <id>.propertyChecked(property, checked) { ... }` | Variant property, bool checked |
| property_deleted | `on <id>.propertyDeleted(property) { ... }` | Variant property |
| property_favorited | `on <id>.propertyFavorited(property, favorited) { ... }` | Variant property, bool favorited |
| property_keyed | `on <id>.propertyKeyed(property) { ... }` | Variant property |
| property_keyed_with_value | `on <id>.propertyKeyedWithValue(property, value) { ... }` | Variant property, Variant value |
| property_overridden | `on <id>.propertyOverridden() { ... }` | - |
| property_pinned | `on <id>.propertyPinned(property, pinned) { ... }` | Variant property, bool pinned |
| resource_selected | `on <id>.resourceSelected(path, resource) { ... }` | string path, Object resource |
| selected | `on <id>.selected(path, focusableIdx) { ... }` | string path, int focusableIdx |

## Runtime Actions

This page lists **callable methods declared by `EditorProperty`**.
Inherited actions are documented in: [Container](Container.md)

| Godot Method | SMS Call | Params | Returns |
|-|-|-|-|
| deselect | `<id>.deselect()` | - | void |
| get_edited_object | `<id>.getEditedObject()` | - | Object |
| get_edited_property | `<id>.getEditedProperty()` | - | Variant |
| is_selected | `<id>.isSelected()` | - | bool |
| is_using_folding | `<id>.isUsingFolding()` | - | bool |
| select | `<id>.select(focusable)` | int focusable | void |
| update_property | `<id>.updateProperty()` | - | void |
