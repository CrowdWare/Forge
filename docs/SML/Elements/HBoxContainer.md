# HBoxContainer

## Inheritance

[HBoxContainer](HBoxContainer.md) → [HBoxContainer](HBoxContainer.md) → [BoxContainer](BoxContainer.md) → [Container](Container.md) → [Control](Control.md) → [CanvasItem](CanvasItem.md) → [Node](Node.md) → [Object](Object.md)

## Derived Classes

### Direct subclasses

- [EditorResourcePicker](EditorResourcePicker.md)
- [EditorToaster](EditorToaster.md)
- [ForgeBgHBoxContainer](ForgeBgHBoxContainer.md)
- [HBoxContainer](HBoxContainer.md)
- [OpenXRInteractionProfileEditorBase](OpenXRInteractionProfileEditorBase.md)

## Properties

This page lists **only properties declared by `HBoxContainer`**.
Inherited properties are documented in: [BoxContainer](BoxContainer.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|
| - | id | identifier | - |
| - | alignment | enum: begin, center, end | begin |
| - | spacing | int | - |
| separation (theme override) | spacing | int | - |
| - (drawn via BgHBoxContainer._Draw) | bgColor | color | - |
| - (drawn via BgHBoxContainer._Draw) | borderColor | color | - |
| - (drawn via BgHBoxContainer._Draw) | borderWidth | int | - |
| - (drawn via BgHBoxContainer._Draw) | borderRadius | int | - |
| StyleBoxFlat.shadow_color | shadowColor | color | - |
| StyleBoxFlat.shadow_size | shadowSize | int | - |
| StyleBoxFlat.shadow_offset.x | shadowOffsetX | int | 0 |
| StyleBoxFlat.shadow_offset.y | shadowOffsetY | int | 0 |
| - (drawn via BgHBoxContainer._Draw) | highlightColor | color | - |
| - (expands to profile properties before build) | elevation | identifier | - |
### Examples

```sml
HBoxContainer {
    alignment: center
    Label { text: "Left" }
    Label { text: "Right" }
}
```

## Events

This page lists **only signals declared by `HBoxContainer`**.
Inherited signals are documented in: [BoxContainer](BoxContainer.md)

| Godot Signal | SMS Event | Params |
|-|-|-|

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

