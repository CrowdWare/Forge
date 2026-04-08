# Task: forgecli build - macOS App & Android APK Export

## Goal

Add a `build` sub-command to `ForgeCli.Native` that packages a Forge project
(SML + SMS files) into a standalone macOS `.app` or Android `.apk` by driving
the Godot 4.6 headless exporter. The user never touches Godot manually; the CLI
downloads what it needs and runs the export automatically.

---

## Supported Platforms

| Platform | Build host | Output |
|---|---|---|
| macOS | macOS | `.zip` containing `<name>.app` |
| Android | macOS (+ Android NDK) | `.apk` |

Linux builds are done natively on a Linux machine - no cross-compilation from macOS.

## New Commands

```
forgecli build mac     --project <dir> [--output <path>] [--godot-version 4.6-stable]
forgecli build android --project <dir> [--output <path>] [--godot-version 4.6-stable]
```

Defaults:
- `--output` mac     → `<project-name>.zip`  (contains `<name>.app`)
- `--output` android → `<project-name>.apk`
- `--godot-version`  → `4.6-stable`

---

## New Source Files

### `ForgeCli.Native/src/godot_sdk.h` + `godot_sdk.cpp`

Handles all Godot binary + template download/cache logic. No Godot dependency -
pure C++17 + system tools (`curl`, `unzip`).

#### Cache layout

```
~/.cache/forge-runner/godot/<version>/
    godot           (the Godot editor binary, chmod +x on macOS/Linux)
    templates/      (extracted export templates, installed to Godot's template dir)
    .ready          (sentinel file - present means binary + templates are ready)
```

Platform-specific Godot template install dir (where Godot expects them):
- macOS: `~/Library/Application Support/Godot/export_templates/<version_normalized>/`
- Linux: `~/.local/share/godot/export_templates/<version_normalized>/`
- Windows: `%APPDATA%\Godot\export_templates\<version_normalized>\`

`version_normalized`: replace `-` with `.`  →  `4.6-stable` → `4.6.stable`

#### Download URLs (GitHub releases)

Base: `https://github.com/godotengine/godot/releases/download/<version>/`

| File | Purpose |
|---|---|
| `Godot_v<version>_macos.universal.zip` | Godot binary (macOS, used as build host) |
| `Godot_v<version>_linux.x86_64.zip` | Godot binary (Linux build host) |
| `Godot_v<version>_win64.exe.zip` | Godot binary (Windows build host) |
| `Godot_v<version>_export_templates.tpz` | Export templates (all platforms, one file) |

The `.tpz` is a ZIP file - use `unzip` to extract it.

#### API

```cpp
namespace godot_sdk {

struct Config {
    std::string version;          // e.g. "4.6-stable"
    std::filesystem::path cache;  // ~/.cache/forge-runner/godot/<version>
};

// Returns the path to the Godot binary, downloading if necessary.
// Prints [DL] progress lines to stdout. Returns "" on failure.
std::string ensure_binary(const Config& cfg, std::string& err);

// Downloads and installs export templates into Godot's template dir.
// Skips if already installed (sentinel .ready present).
bool ensure_export_templates(const Config& cfg, std::string& err);

// Returns the normalized version string ("4.6.stable" from "4.6-stable").
std::string normalize_version(const std::string& version);

// Returns the Godot export_templates install dir for the current platform.
std::filesystem::path template_install_dir(const std::string& version_normalized);

} // namespace godot_sdk
```

#### Implementation notes

- Use `system("curl -L -# -o <dst> <url>")` for downloads (curl is always present on macOS; on Linux require it; on Windows use `wininet` or document curl as dep).
- Use `system("unzip -q <src> -d <dst>")` for extraction.
- After extracting the `.tpz`, the templates are in `templates/` subfolder inside the archive. Copy that subfolder's contents to `template_install_dir`.
- After extracting the macOS binary zip, the binary is at `Godot.app/Contents/MacOS/Godot`. Copy it to `cache/godot` and `chmod +x`.
- After all steps succeed, write the `.ready` sentinel.

---

### `ForgeCli.Native/src/cmd_build.cpp`

Implements `cmd_build(platform, args)`.

#### Flow

```
1.  Parse args: --project, --output, --godot-version, --native-lib-dir, --host-project-dir
2.  Validate project (reuse validate logic - call parse_sml / parse_sms internally)
3.  godot_sdk::ensure_binary()       → godot_path
4.  godot_sdk::ensure_export_templates()
5.  Determine project name from --project dir name
6.  Create staging dir in system temp: forge-build-<uuid>/
7.  Copy ForgeRunner.Native host project into staging/
8.  Copy user SML/SMS files into staging/ root (alongside project.godot)
9.  Copy GDExtension library into staging/ root
10. Write export_presets.cfg into staging/
11. Run: <godot_path> --headless --export-release "<preset>" <output>
12. Remove staging dir
13. Print [OK] <output>
```

#### Environment variables / flags

| Env var | Flag | Purpose |
|---|---|---|
| `FORGE_HOST_PROJECT_DIR` | `--host-project-dir` | Path to `ForgeRunner.Native/host/` |
| `FORGE_NATIVE_LIB_DIR` | `--native-lib-dir` | Dir containing `libforge_runner_native.dylib` etc. |

Both are required. The CLI exits with a clear error if missing.

#### Staging directory layout

```
staging/
    project.godot                      (copied from host, then patched)
    main.tscn                          (copied from host)
    icon.svg                           (copied from host)
    forge_runner_native.gdextension    (copied from host)
    libforge_runner_native.dylib       (or .so / .dll - copied from native-lib-dir)
    app.sml                            (from user project)
    main.sml                           (from user project)
    main.sms                           (from user project)
    theme.sml                          (from user project, if present)
    strings.sml                        (from user project, if present)
    assets/                            (from user project assets/, if present)
    export_presets.cfg                 (generated)
```

Note: Copy all `.sml`, `.sms` files from user project root + the `assets/` dir.

#### project.godot patching

After copying, patch `project.godot` to set the application name to the project name:

Find line: `config/name=` → replace value with the project name.

Use simple line-by-line string replacement (no regex needed).

#### export_presets.cfg - macOS

```ini
[preset.0]

name="macOS"
platform="macOS"
runnable=true
advanced_options=false
dedicated_server=false
custom_features=""
export_filter="all_resources"
include_filter=""
exclude_filter=""
export_path="<output_path>"
encryption_include_filters=""
encryption_exclude_filters=""
encrypt_pck=false
encrypt_directory=false

[preset.0.options]

binary_format/architecture="universal"
custom_template/debug=""
custom_template/release=""
debug/export_console_wrapper=1
codesign/enable=false
codesign/identity=""
codesign/entitlements/custom_file=""
notarization/enable=false
privacy/camera_usage_description=""
privacy/microphone_usage_description=""
application/bundle_identifier="io.crowdware.forgerunner"
application/short_version="1.0"
application/version="1.0"
application/copyright="CrowdWare"
```

Replace `<output_path>` with the resolved `--output` path.

#### export_presets.cfg - Android

```ini
[preset.0]

name="Android"
platform="Android"
runnable=true
advanced_options=false
dedicated_server=false
custom_features=""
export_filter="all_resources"
include_filter=""
exclude_filter=""
export_path="<output_path>"
encryption_include_filters=""
encryption_exclude_filters=""
encrypt_pck=false
encrypt_directory=false

[preset.0.options]

custom_template/debug=""
custom_template/release=""
gradle_build/use_gradle_build=false
gradle_build/export_format=0
package/unique_name="io.crowdware.<project_name_lower>"
package/name="<project_name>"
package/signed=false
launcher_icons/main_192x192=""
version/code=1
version/name="1.0"
user_data_folder="io.crowdware.<project_name_lower>"
```

Replace `<output_path>`, `<project_name>`, `<project_name_lower>` accordingly.

#### Android prerequisites check

Before starting, verify:
- `adb` in PATH (Android SDK) - print warning if missing, but don't abort
- `java` in PATH (JDK 17+) - print warning if missing

Android export will fail at the Godot step if these are missing; let Godot's error
message speak for itself. The CLI should only pre-check and warn, not hard-block.

---

## Samples

### `samples/mac_demo/`

A minimal Forge app for verifying macOS export. No 3D, no docking - just a clean
Window to prove the round-trip works.

**`app.sml`**
```sml
SplashScreen {
    id: splashScreen
    size: 640, 360
    duration: 800
    loadOnReady: "main.sml"

    VBoxContainer {
        anchors: left | top | right | bottom

        Control { sizeFlagsVertical: expandFill }

        Label {
            text: "Loading Mac Demo..."
            sizeFlagsHorizontal: shrinkCenter
        }

        Control { sizeFlagsVertical: expandFill }
    }
}
```

**`main.sml`**
```sml
Window {
    id: mainWindow
    title: "Mac Demo"
    minSize: 640, 400
    size: 800, 500

    VBoxContainer {
        anchors: left | top | right | bottom
        padding: 40, 40, 40, 40

        Label {
            id: heading
            text: "Hello from Forge on macOS"
            fontSize: 24
        }

        Label {
            id: subline
            text: "This app was built with forgecli build mac."
            fontSize: 14
        }

        Control { sizeFlagsVertical: expandFill }

        Button {
            id: btnQuit
            text: "Quit"
            sizeFlagsHorizontal: shrinkCenter
        }
    }
}
```

**`main.sms`**
```sms
fun ready() {
    log.info("mac_demo ready")
}

fun btnQuit.clicked() {
    os.quit()
}
```

**`theme.sml`**
```sml
Colors {
    accent: "#007AFF"
}
```

**`strings.sml`**
```sml
Strings {
    windowTitle: "Mac Demo"
}
```

**`README.md`**
```markdown
# mac_demo

Minimal Forge app for verifying macOS export.

## Build

```bash
FORGE_HOST_PROJECT_DIR=../../ForgeRunner.Native/host \
FORGE_NATIVE_LIB_DIR=../../ForgeRunner.Native/build \
forgecli build mac --project . --output mac_demo.zip
```

Unzip `mac_demo.zip` and run `Mac Demo.app`.
```

---

### `samples/android_demo/`

A minimal Forge app for the Android vertical slice.

**`app.sml`**
```sml
SplashScreen {
    id: splashScreen
    size: 360, 640
    duration: 800
    loadOnReady: "main.sml"

    VBoxContainer {
        anchors: left | top | right | bottom

        Control { sizeFlagsVertical: expandFill }

        Label {
            text: "Loading..."
            sizeFlagsHorizontal: shrinkCenter
        }

        Control { sizeFlagsVertical: expandFill }
    }
}
```

**`main.sml`**
```sml
Window {
    id: mainWindow
    title: "Android Demo"
    size: 360, 640

    VBoxContainer {
        anchors: left | top | right | bottom
        padding: 32, 32, 32, 32

        Label {
            id: heading
            text: "Hello from Forge on Android"
            fontSize: 20
        }

        Label {
            id: subline
            text: "Vertical slice - build with forgecli build android."
            fontSize: 13
        }

        Control { sizeFlagsVertical: expandFill }

        Button {
            id: btnTest
            text: "Tap me"
            sizeFlagsHorizontal: shrinkCenter
        }

        Label {
            id: lblResult
            text: ""
            sizeFlagsHorizontal: shrinkCenter
        }
    }
}
```

**`main.sms`**
```sms
var tapCount = 0

fun ready() {
    log.info("android_demo ready")
}

fun btnTest.clicked() {
    tapCount = tapCount + 1
    var label = ui.getObject("lblResult")
    label.text = "Tapped " + tapCount + " times"
}
```

**`theme.sml`**
```sml
Colors {
    accent: "#3DDC84"
}
```

**`strings.sml`**
```sml
Strings {
    windowTitle: "Android Demo"
}
```

**`README.md`**
```markdown
# android_demo

Minimal Forge app for verifying Android export (vertical slice).

## Prerequisites

- Android SDK with `adb` in PATH
- JDK 17+ with `java` in PATH
- Godot Android export templates (downloaded automatically)

## Build

```bash
FORGE_HOST_PROJECT_DIR=../../ForgeRunner.Native/host \
FORGE_NATIVE_LIB_DIR=../../ForgeRunner.Native/build \
forgecli build android --project . --output android_demo.apk
```

Install on connected device:

```bash
adb install android_demo.apk
```
```

---

## CMakeLists.txt changes

Add `cmd_build.cpp` and `godot_sdk.cpp` to the `forgecli-native` executable sources:

```cmake
add_executable(forgecli-native
    src/main.cpp
    src/sandbox_policy.cpp
    src/cmd_build.cpp
    src/godot_sdk.cpp
)
```

---

## main.cpp changes

- Add `#include "cmd_build.h"` (or inline the `cmd_build` declaration)
- In `main()`, add dispatch: `if (cmd == "build") return cmd_build(args);`
- In `help()`, add:
  ```
    forgecli-native build mac     --project <dir> [--output <path>] [--godot-version <ver>]
    forgecli-native build android --project <dir> [--output <path>] [--godot-version <ver>]
  ```

---

## .gitignore additions

Add to repo root `.gitignore`:

```
# Godot SDK cache (downloaded by forgecli build)
# (these live in ~/.cache/forge-runner/godot/ - already outside repo)

# Build staging
/tmp/forge-build-*/
```

No in-repo ignores needed for the Godot binary/templates since they land in `~/.cache`.

---

## Acceptance Criteria

- [ ] `forgecli build mac --project samples/mac_demo` produces a `.zip` containing a runnable `Mac Demo.app` on macOS
- [ ] `forgecli build android --project samples/android_demo` produces an `.apk` installable via `adb install`
- [ ] First run downloads Godot binary + templates and prints `[DL]` progress; subsequent runs skip download (`.ready` sentinel)
- [ ] Missing `FORGE_HOST_PROJECT_DIR` or `FORGE_NATIVE_LIB_DIR` → clear error, exit 1
- [ ] Invalid SML/SMS in project → validate error printed, export aborted, exit 2
- [ ] `forgecli help` shows the new `build` commands
- [ ] `samples/mac_demo/` and `samples/android_demo/` present with all required files
