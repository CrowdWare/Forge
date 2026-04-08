# Native Splash → Main Window Transition

> Anmerkung von Art: Ich glaube die Transition haben wir bereits vorgezogen, weil es mich gestört hatte.

## Goal
Port the two-window Splash → Main pattern from `Main.cs` to `ForgeRunner.Native`
so that the root window shows a borderless splash screen, then transitions to a
full app window without the root-window-visibility problem.

## Context
Godot's root window (ID 0) cannot be hidden (`Can't change visibility of main window`).
The C# solution:
1. Splash runs in the root window (borderless, sized to splash dimensions).
2. Main UI is built invisibly in a child `Window` node.
3. After `InvokeReady`, the child window is shown; the root window is shrunk to 1×1
   and moved to off-screen (−32000, −32000).

In `forge_runner_main.cpp` only a simple timer-based sequential SML swap exists.
There is no proper two-window setup.

## Implementation

### Splash phase
- Root window: `FLAG_BORDERLESS = true`, sized to splash SML `width`/`height`.
- `splash_load_on_ready:` property triggers main load after `splash_duration_ms`.

### Main phase
1. Create a child `Window` with `visible = false`.
2. Build main SML UI into the child window's root Control.
3. After build + SMS ready: `child_win->set_visible(true)`.
4. Shrink root window: `get_window()->set_size({1,1})`; `set_position({-32000,-32000})`.
5. Connect `child_win->close_requested` → `get_tree()->quit()`.

### Window config (from SML)
SML `Window` root node properties already parsed by `apply_window_props()`:
- `title`, `width`, `height`, `minWidth`, `minHeight`, `extendToTitle`
- Add: `borderless` (bool) → `FLAG_BORDERLESS`
- Add: `alwaysOnTop` (bool) → `FLAG_ALWAYS_ON_TOP`

### SwapToUiAsync equivalent
`ForgeRunnerNativeMain::swap_to_main(path)`:
1. Detach current content from root.
2. Create child Window.
3. Build new UI into child Window.
4. Run SMS script (if `.sms` file present beside `.sml`).
5. Show child window.
6. Hide root window.

## Acceptance Criteria
- Splash displays for the configured duration with no visible root window chrome.
- After splash, the main app window appears as a proper OS window.
- Closing the main window quits the application.
- Borderless + extendToTitle flags work on macOS.

## Reference
- C#: `ForgeRunner/Main.cs` (`SwapToUiAsync`, `AttachUi`, `DetachUi`)
- C#: `MEMORY.md` - "Architektur: Zwei-Fenster Splash/Main" section
