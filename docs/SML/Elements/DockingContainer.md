# DockingContainer

## Inheritance

[DockingContainer](DockingContainer.md) → [PanelContainer](PanelContainer.md) → [Container](Container.md) → [Control](Control.md) → [CanvasItem](CanvasItem.md) → [Node](Node.md) → [Object](Object.md)

## Properties

This page lists **only properties declared by `DockingContainer`**.
Inherited properties are documented in: [PanelContainer](PanelContainer.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|
| - | id | identifier | - |
| - | dockSide | enum | center |
| - | fixedWidth | int | 240 |
| - | minFixedWidth | int | 140 |
| - | fixedHeight | int | - |
| - | minHeight | int | - |
| - | minFixedHeight | int | 80 |
| - | heightPercent | float | 50 |
| - | flex | bool | false |
| - | collapsed | bool | false |
| - | closeable | bool | true |
| - | dragToRearrangeEnabled | bool | true |
| - | tabsRearrangeGroup | int | 1 |

> Automatically creates an internal TabContainer.
> Direct child controls become tabs. Use the attached property syntax to define tab captions: `<containerId>.title: "Caption"` or `DockingContainer.title: "Caption"`.
> Attached properties are resolved by instance id first, then by type name. The qualifier must refer to the direct parent DockingContainer.
> dockSide supports: farLeft, farLeftBottom, left, leftBottom, center, right, rightBottom, farRight, farRightBottom.
> For split columns (top/bottom), height can be controlled via fixedHeight (px) or heightPercent (0..100).
> Priority: fixedHeight > heightPercent > automatic 50/50 fallback.
> When flex is true, fixedWidth is ignored (warning emitted). Use minWidth to enforce a lower bound.
> collapsed:true hides the docking container from layout until shown or moved into.
> Use enum syntax without quotes, e.g. dockSide: left.
> dragToRearrangeEnabled: false excludes this container from docking move targets (kebab menu).
> A container is not listed as move target for itself (same dock slot is filtered).

### Examples

```sml
DockingContainer {
    id: leftDock
    dockSide: left
    fixedWidth: 280
    dragToRearrangeEnabled: true
    tabsRearrangeGroup: 1

    VBoxContainer {
        id: project
        leftDock.title: "Project"
    }

    VBoxContainer {
        id: search
        leftDock.title: "Search"
    }
}

// Alternatively qualify by type name:
DockingContainer {
    id: rightDock
    dockSide: right
    fixedWidth: 360

    VBoxContainer {
        id: inspector
        DockingContainer.title: "Inspector"
    }
}
```

## Events

This page lists **only signals declared by `DockingContainer`**.
Inherited signals are documented in: [PanelContainer](PanelContainer.md)

| Godot Signal | SMS Event | Params |
|-|-|-|
