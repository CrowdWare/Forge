# ForgeCli.Native

Minimal C++ rewrite of ForgeCli.

Current commands:
- `new`
- `validate` (via `smlcore_native` + `sms_native`)
- `build` (`mac` or `android`, via Godot headless export)
- `toolchain doctor` (checks required local tools + native lib env)
- `sms run` (execute `.sms` directly via interpreter)
- `sms llvm-ir` (emit LLVM IR from `.sms`)
- `sms build` (vertical slice: `.sms -> native binary` via LLVM IR + clang)
- `sms demo` (writes `main.sms`, runs it, optional native build)

Sandbox policy during `validate`:
- Registers `sms_native_set_sandbox_path_callback(...)`.
- Allows only `res:/`, `appRes:/`, `user:/`.
- Resolves `res:/` + `appRes:/` to project root and `user:/` to `<project>/.forge_user`.
- Rejects traversal and symlink-based escape attempts via canonical containment checks.

Build:

```bash
cmake -S ForgeCli.Native -B ForgeCli.Native/build -DCMAKE_BUILD_TYPE=Release
cmake --build ForgeCli.Native/build --config Release
```

Run:

```bash
SML_NATIVE_LIB_DIR="$(pwd)/SMLCore.Native/build" \
SMS_NATIVE_LIB_DIR="$(pwd)/SMSCore.Native/build" \
./ForgeCli.Native/build/forgecli-native validate --project ./MyApp
```

Toolchain doctor:

```bash
SML_NATIVE_LIB_DIR="$(pwd)/SMLCore.Native/build" \
SMS_NATIVE_LIB_DIR="$(pwd)/SMSCore.Native/build" \
./ForgeCli.Native/build/forgecli-native toolchain doctor
```

One-command SMS demo (writes + runs `main.sms`):

```bash
SMS_NATIVE_LIB_DIR="$(pwd)/SMSCore.Native/build" \
./ForgeCli.Native/build/forgecli-native sms demo
```

One-command SMS demo with native binary:

```bash
SMS_NATIVE_LIB_DIR="$(pwd)/SMSCore.Native/build" \
./ForgeCli.Native/build/forgecli-native sms demo --build --out ./main
./main
```

SMS run (interpreter):

```bash
SMS_NATIVE_LIB_DIR="$(pwd)/SMSCore.Native/build" \
./ForgeCli.Native/build/forgecli-native sms run ./main.sms
```

SMS to native binary (`.sms -> .exe` on Windows / native binary on macOS/Linux):

```bash
SMS_NATIVE_LIB_DIR="$(pwd)/SMSCore.Native/build" \
./ForgeCli.Native/build/forgecli-native sms build ./main.sms --out ./main
./main
```

Note: For `sms build`, the LLVM backend accepts either:
- a top-level function call (for example `main()`)
- or a `fun main() { ... }` fallback entry function

Build (macOS zip):

```bash
SML_NATIVE_LIB_DIR="$(pwd)/SMLCore.Native/build" \
SMS_NATIVE_LIB_DIR="$(pwd)/SMSCore.Native/build" \
FORGE_HOST_PROJECT_DIR="$(pwd)/ForgeRunner.Native/host" \
FORGE_NATIVE_LIB_DIR="$(pwd)/ForgeRunner.Native/build" \
./ForgeCli.Native/build/forgecli-native build mac --project ./samples/mac_demo --output ./mac_demo.zip
```

Build (Android apk):

```bash
SML_NATIVE_LIB_DIR="$(pwd)/SMLCore.Native/build" \
SMS_NATIVE_LIB_DIR="$(pwd)/SMSCore.Native/build" \
FORGE_HOST_PROJECT_DIR="$(pwd)/ForgeRunner.Native/host" \
FORGE_NATIVE_LIB_DIR="/abs/path/to/android/libs" \
./ForgeCli.Native/build/forgecli-native build android --project ./samples/android_demo --output ./android_demo.apk --android-package-id com.example.myapp
```

Install APK on device:

```bash
adb install -r ./android_demo.apk
```

Android build note:
- During `build android`, ForgeCli generates SMS LLVM IR artifacts for project-root `.sms` files and stages them under `sms_llvm/*.ll` in the export project.
- LLVM codegen supports explicit compiler mode switch: `exe` (requires entry/main) and `lib` (module/library generation for event-driven scripts).
- Package id can be provided externally via `--android-package-id <id>` or `FORGE_ANDROID_PACKAGE_ID`.
- Android version code can be overridden via `FORGE_ANDROID_VERSION_CODE`.
- Android packaging is native-strict: build fails if any `.sms` cannot be compiled to LLVM IR.

Run sandbox tests:

```bash
cmake -S ForgeCli.Native -B ForgeCli.Native/build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build ForgeCli.Native/build --config Release
ctest --test-dir ForgeCli.Native/build --output-on-failure
```
