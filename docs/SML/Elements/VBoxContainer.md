# VBoxContainer

## Inheritance

[VBoxContainer](VBoxContainer.md) → [VBoxContainer](VBoxContainer.md) → [BoxContainer](BoxContainer.md) → [Container](Container.md) → [Control](Control.md) → [CanvasItem](CanvasItem.md) → [Node](Node.md) → [Object](Object.md)

## Derived Classes

### Direct subclasses

- [ColorPicker](ColorPicker.md)
- [ForgeBgVBoxContainer](ForgeBgVBoxContainer.md)
- [ForgeMarkdownContainer](ForgeMarkdownContainer.md)
- [Markdown](Markdown.md)
- [ScriptEditorBase](ScriptEditorBase.md)
- [VBoxContainer](VBoxContainer.md)

## Properties

This page lists **only properties declared by `VBoxContainer`**.
Inherited properties are documented in: [BoxContainer](BoxContainer.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|
| - | id | identifier | - |
| - | alignment | enum: begin, center, end | begin |
| - | spacing | int | - |
| separation (theme override) | spacing | int | - |
| - (drawn via BgVBoxContainer._Draw) | bgColor | color | - |
| - (drawn via BgVBoxContainer._Draw) | borderColor | color | - |
| - (drawn via BgVBoxContainer._Draw) | borderWidth | int | - |
| - (drawn via BgVBoxContainer._Draw) | borderRadius | int | - |
| StyleBoxFlat.shadow_color | shadowColor | color | - |
| StyleBoxFlat.shadow_size | shadowSize | int | - |
| StyleBoxFlat.shadow_offset.x | shadowOffsetX | int | 0 |
| StyleBoxFlat.shadow_offset.y | shadowOffsetY | int | 0 |
| - (drawn via BgVBoxContainer._Draw) | highlightColor | color | - |
| - (expands to profile properties before build) | elevation | identifier | - |
### Examples

```sml
VBoxContainer {
    alignment: center
    Label { text: "Top" }
    Label { text: "Bottom" }
}
```

## Events

This page lists **only signals declared by `VBoxContainer`**.
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

