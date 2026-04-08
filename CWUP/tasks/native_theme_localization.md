# Native Theme + Localization System

## Goal
Port `ThemeStore.cs` and `LocalizationStore.cs` to C++ so that SML properties
`@Colors.*`, `@Layouts.*`, and `@Strings.*` resolve correctly in
`ForgeRunner.Native`.

## Context
`UiBuilder::load_strings()` already loads `strings.sml` into a map. That is the
localization half - but it needs to be extended to full spec parity. The theme
half (`@Colors.*`, `@Layouts.*`) is not implemented at all in C++.

## Subsystems to Port

### ThemeStore
Load `theme.sml` from the app directory. Expected structure:

```
Colors {
    primary: "#1e88e5"
    surface: "#1a1a1a"
    ...
}
Layouts {
    gap: 8
    radius: 6
    ...
}
Elevations {
    card {
        bgColor: @Colors.surface
        borderColor: "#333"
        borderRadius: @Layouts.radius
        shadowSize: 4
        shadowColor: "#0006"
    }
}
```

- `@Colors.key` → hex color string
- `@Layouts.key` → int or float
- `@Elevations.name` → expands into multiple properties inline before build

### Localization
- `strings.sml` already partially loaded.
- Add language fallback: e.g. `strings.de.sml` → fallback to `strings.sml`.
- `@Strings.key` → resolved string (already used as `resolve_text()` in UiBuilder).

### Elevation Expansion
Before a node is built, if it has an `elevation: name` property, look up the
`Elevations.name` block and inject all its properties into the node - identical
to the C# pre-build pass in `SmlUiBuilder`.

### Resource Reference Resolution
Generalise `UiBuilder::resolve_text()` into `UiBuilder::resolve_ref(SmlValue)`
that handles `@Colors.*`, `@Layouts.*`, `@Strings.*`, and `@Icons.*`.

## Acceptance Criteria
- `@Colors.*`, `@Layouts.*`, `@Strings.*` refs resolve to their values in all
  property setters.
- `elevation: card` on a Panel expands to bgColor / borderColor / borderRadius /
  shadowSize / shadowColor before the node is built.
- Missing key → warning, no crash.

## Reference
- C#: `ForgeRunner/Runtime/UI/ThemeStore.cs`
- C#: `ForgeRunner/Runtime/UI/LocalizationStore.cs`
- C#: `ForgeRunner/Runtime/UI/SmlUiBuilder.cs` (elevation expansion)
