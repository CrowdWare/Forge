# Tutorial: Theme Demo (Mac)

A minimal Forge app demonstrating day/night theme switching and a button-triggered animation.

## What this tutorial covers

- `ui.setTheme(mode)` — switch between `"light"` and `"dark"` token sets at runtime
- `ui.animate(id, property, toValue, durationMs)` — tween a UI element property

## Run (interpreter mode — no compilation)

```bash
forgecli run mac --project tutorials/theme-demo-mac
```

Starts the app immediately via the ForgeRunner host project.
Godot is downloaded automatically on first use.
Use this while developing — changes to SML and SMS files are picked up on the next run.

## Build (ship-ready .dmg)

```bash
forgecli build mac --project tutorials/theme-demo-mac
```

Compiles SML + SMS to a standalone `.dmg` with the native ForgeRunner embedded.

## Typical workflow

1. `forgecli run mac --project tutorials/theme-demo-mac` — iterate and preview
2. `forgecli build mac --project tutorials/theme-demo-mac` — ship when ready
