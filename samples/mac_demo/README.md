# mac_demo

Minimal Forge app for verifying macOS export.

## Build

```bash
FORGE_HOST_PROJECT_DIR=../../ForgeRunner.Native/host \
FORGE_NATIVE_LIB_DIR=../../ForgeRunner.Native/build \
forgecli build mac --project . --output mac_demo.zip
```

Unzip `mac_demo.zip` and run `Mac Demo.app`.
