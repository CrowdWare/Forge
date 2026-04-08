# Task: Add `bgColor` Property to SML DockingContainer

## Background

`DockingContainerControl` is a custom Godot `PanelContainer` that renders its background
via `_Draw()` using a color read from the Godot theme system. However, Godot applies
gamma/sRGB correction when reading colors from the theme, making it impossible to achieve
a pixel-exact color match with the app titlebar.

The solution is to pass the background color directly as a meta value on the node,
bypassing the theme pipeline entirely. The color is then read in `_Draw()` via
`GetMeta("bgColor")` and converted to a `Color` using `new Color(hexString)`.

## Goal

Add a `bgColor` property to the SML `DockingContainer` element so that the background
color can be defined directly in the SML markup:

```
DockingContainer {
    id: farLeftDock
    bgColor: #1C1E24
    dockSide: left
    fixedWidth: 300
}
```

## Requirements

- The property name is `bgColor`
- The value is a hex color string, e.g. `#1C1E24`
- The value must be stored as a Godot Meta on the `DockingContainerControl` node
- The meta key must be `bgColor`
- The C# code in `DockingContainerControl._Draw()` already reads this meta:

```csharp
public override void _Draw()
{
    Color bgColor;
    if (HasMeta("bgColor"))
    {
        bgColor = new Color(GetMeta("bgColor").AsString());
    }
    else
    {
        bgColor = GetThemeColor("dock_background", "DockingContainerControl");
    }
    DrawRect(new Rect2(Vector2.Zero, Size), bgColor);
}
```

## Steps

1. Find the SML parser/mapper that handles `DockingContainer` properties
2. Add `bgColor` as a supported property
3. Parse the hex string value and store it via `SetMeta("bgColor", value)` on the node
4. Add `bgColor` to the property documentation for `DockingContainer`
5. Write a unit test that verifies the meta is set correctly when `bgColor` is present
6. Write a unit test that verifies `_Draw()` uses the meta color when present and falls
   back to the theme color when absent

## Notes

- Do **not** use `new Color(hexString)` in the SML parser - just store the raw hex string
  as meta and let `DockingContainerControl._Draw()` handle the conversion
- The fallback (theme color) should remain functional for cases where `bgColor` is not set
- Follow the existing patterns for property mapping and documentation in this codebase
