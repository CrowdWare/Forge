# ForgeCli.Native

Minimal C++ rewrite of ForgeCli.

Current commands:
- `new`
- `validate` (via `smlcore_native` + `sms_native`)
- `build` (`mac` or `android`, via Godot headless export)

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

Build (macOS zip):

```bash
SML_NATIVE_LIB_DIR="$(pwd)/SMLCore.Native/build" \
SMS_NATIVE_LIB_DIR="$(pwd)/SMSCore.Native/build" \
FORGE_HOST_PROJECT_DIR="$(pwd)/ForgeRunner.Native/host" \
FORGE_NATIVE_LIB_DIR="$(pwd)/ForgeRunner.Native/build" \
./ForgeCli.Native/build/forgecli-native build mac --project ./samples/mac_demo --output ./mac_demo.zip
```

Run sandbox tests:

```bash
cmake -S ForgeCli.Native -B ForgeCli.Native/build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build ForgeCli.Native/build --config Release
ctest --test-dir ForgeCli.Native/build --output-on-failure
```
