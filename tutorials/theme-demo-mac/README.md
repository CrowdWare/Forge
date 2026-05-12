# Tutorial: Theme Demo (Mac)

A minimal Forge app demonstrating day/night theme switching and a button-triggered animation.

## What this tutorial covers

- `ui.setTheme(mode)` — switch between `"light"` and `"dark"` token sets at runtime
- `ui.animate(id, property, toValue, durationMs)` — tween a UI element property

## Build

```bash
forgecli build mac --project tutorials/theme-demo-mac
```

## Runtime gaps (to be implemented)

| Function | Status |
|---|---|
| `ui.setTheme(mode)` | not yet in runtime bridge |
| `ui.animate(id, property, toValue, durationMs)` | not yet in runtime bridge |
