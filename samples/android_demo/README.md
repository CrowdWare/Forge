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
