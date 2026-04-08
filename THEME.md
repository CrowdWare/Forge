# THEME Guide

The ForgeRunner theme is defined entirely in SML and generated into a Godot theme file.

- **Source of truth:** `ForgeRunner/theme.sml`
- **Generated output:** `ForgeRunner/theme.tres`
- **Loaded at runtime:** `Main.cs` via `GD.Load<Theme>("res://theme.tres")`

---

## Workflow

### Changing design tokens

1. Edit `ForgeRunner/theme.sml` - all colors and layout values are defined here as named tokens.
2. Regenerate `theme.tres`:

   ```bash
   ./run_runner.sh theme
   ```

3. Run the app and compare visually.

Do **not** edit `theme.tres` directly - it is generated and will be overwritten.

---

## Token overview

### Colors

| Token | Role |
|---|---|
| `accent` | Primary highlight (buttons pressed, tabs selected, carets, selections) |
| `focus` | Focus ring outline |
| `textPrimary` | Default button/label text |
| `textHover` | Button text on hover |
| `textPressed` | Button/tab text when active |
| `textDisabled` | Dimmed text |
| `textInput` | Text in LineEdit / TextEdit |
| `textPlaceholder` | Placeholder text in LineEdit |
| `textList` | Text in Tree / ItemList |
| `textTabDisabled` | Disabled tab label |
| `textTabHover` | Hovered tab label |
| `textTabUnselected` | Inactive tab label |
| `bgDark` | Darkest surface (tree panels, tab backgrounds, text editors) |
| `bgBase` | Input field background |
| `bgMid` | Panel background |
| `bgPanel` | PanelContainer / dock background |
| `bgButtonNormal` | Button resting state |
| `bgButtonHover` | Button hover state |
| `bgButtonDisabled` | Button disabled state |
| `bgInputFocus` | Input field focused state |
| `borderMuted` | Subtle border (disabled button) |
| `borderNormal` | Default button border |
| `borderHover` | Button border on hover |
| `borderInput` | Input field border |
| `borderTabHover` | Hovered tab border |
| `borderTabUnselected` | Inactive tab border |
| `borderPanel` | Tree panel border |
| `borderPanel2` | Panel / TabContainer border |
| `borderPanelContainer` | PanelContainer border |
| `borderTreeGuide` | Tree guide / relationship lines |
| `selectionGreen` | Text selection in TextEdit |

### Layouts

| Token | Role |
|---|---|
| `buttonPaddingH` | Button horizontal content margin |
| `buttonPaddingV` | Button vertical content margin |
| `inputPaddingH` | Input field horizontal content margin |
| `inputPaddingV` | Input field vertical content margin |
| `cornerRadius` | Standard corner radius |
| `cornerRadiusLarge` | Large corner radius (PanelContainer) |
| `tabPaddingH` | Tab horizontal content margin |
| `tabPaddingV` | Tab vertical content margin |
| `fontSize` | Default font size |
| `fontSizeTree` | Tree font size |

---

## App-level overrides

Apps can provide their own `theme.sml` next to `app.sml` to override individual tokens:

```sml
// docs/MyApp/theme.sml
Colors {
    accent: "#FF6B35"
}
```

Only listed tokens are overridden - everything else falls back to the ForgeRunner default.

**Resolution order (highest priority first):**
1. Inline `Colors {}` / `Layouts {}` block inside the SML document
2. App-local `theme.sml` (same directory as `app.sml`)
3. `ForgeRunner/theme.sml` (built-in default)

---

## Validation checklist

After changing tokens, verify:

- Normal / hover / pressed / disabled button states
- Focus ring visibility on dark backgrounds
- Text readability and contrast
- Selected tab and selected list/tree row
- Caret and selection colors in text inputs
