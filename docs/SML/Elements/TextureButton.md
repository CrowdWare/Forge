# TextureButton

## Inheritance

[TextureButton](TextureButton.md) → [TextureButton](TextureButton.md) → [BaseButton](BaseButton.md) → [Control](Control.md) → [CanvasItem](CanvasItem.md) → [Node](Node.md) → [Object](Object.md)

## Derived Classes

### Direct subclasses

- [TextureButton](TextureButton.md)

## Properties

This page lists **only properties declared by `TextureButton`**.
Inherited properties are documented in: [BaseButton](BaseButton.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|
| - | id | identifier | - |
| - | textureNormal | string(url) | "" |
| - | textureHover | string(url) | "" |
| - | texturePressed | string(url) | "" |
| - | textureDisabled | string(url) | "" |
| - | textureFocused | string(url) | "" |
| - | disabled | bool | false |
| - | ignoreTextureSize | bool | false |

> A button that displays textures for each interaction state instead of a text label.
> All texture properties accept the same path formats as TextureRect: res:/, user:/, appRes:/, file://, or absolute paths.

### Examples

```sml
TextureButton {
    id: myBtn
    textureNormal:  "appRes:/assets/icons/bell.png"
    textureHover:   "appRes:/assets/icons/bell_hover.png"
    texturePressed: "appRes:/assets/icons/bell_pressed.png"
}
```

## Events

This page lists **only signals declared by `TextureButton`**.
Inherited signals are documented in: [BaseButton](BaseButton.md)

| Godot Signal | SMS Event | Params |
|-|-|-|

## Runtime Actions

This page lists **callable methods declared by `TextureButton`**.
Inherited actions are documented in: [BaseButton](BaseButton.md)

| Godot Method | SMS Call | Params | Returns |
|-|-|-|-|
| get_click_mask | `<id>.getClickMask()` | - | Object |
| is_flipped_h | `<id>.isFlippedH()` | - | bool |
| is_flipped_v | `<id>.isFlippedV()` | - | bool |

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

