# PanelContainer

## Inheritance

[PanelContainer](PanelContainer.md) → [Container](Container.md) → [Control](Control.md) → [CanvasItem](CanvasItem.md) → [Node](Node.md) → [Object](Object.md)

## Derived Classes

### Direct subclasses

- [DockingContainer](DockingContainer.md)
- [OpenXRBindingModifierEditor](OpenXRBindingModifierEditor.md)
- [ScriptEditor](ScriptEditor.md)

## Properties

This page lists **only properties declared by `PanelContainer`**.
Inherited properties are documented in: [Container](Container.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|
| panel StyleBoxFlat (theme override) | bgColor | color | - |
| panel StyleBoxFlat (theme override) | borderColor | color | - |
| panel StyleBoxFlat (theme override) | borderWidth | int | - |
| panel StyleBoxFlat (theme override) | borderRadius | int | - |

## Events

This page lists **only signals declared by `PanelContainer`**.
Inherited signals are documented in: [Container](Container.md)

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

