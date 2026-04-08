# SMS AOT - Memory Leak Validation

## Status

Open (2026-04-08)

## Goal

Prove that the SMS AOT runtime has zero memory leaks. A clean AddressSanitizer
run is the deliverable - this is the technical basis for the performance story
(no VM, no GC, no leaks).

## Motivation

SMS AOT compiles to native LLVM IR with no interpreter, no JVM, no .NET runtime,
no garbage collector. If we can also prove zero leaks, SMS beats Kotlin and C# on
every memory-safety and performance axis simultaneously.

## Scope

Check the following areas for leaks:

1. **`g_llvm_string_arena` / `g_llvm_array_arena`** - are they cleared between sessions?
2. **`g_aot_lib_handles`** - are all `dlopen` handles closed via `sms_native_aot_lib_close`?
3. **SMS session arenas** - are interpreter-path Value arenas freed on `dispose_session`?
4. **AOT compiled `.so` files** - temp files in the cache dir, lifetime management

## Execution Plan

1. Add ASan flags to `SMSCore.Native` test targets in `CMakeLists.txt`:
   ```cmake
   target_compile_options(<target> PRIVATE -fsanitize=address)
   target_link_options(<target> PRIVATE -fsanitize=address)
   ```
2. Run `sms_native_compiler_codegen_tests` and `sms_native_spec_tests` under ASan
3. Fix any reported leaks
4. Run `forge_runner_native_sms_integration_tests` under ASan
5. Fix any reported leaks
6. Document the clean run as proof

## Files

- `SMSCore.Native/src/sms_native.cpp` - arenas, AOT lib registry
- `SMSCore.Native/CMakeLists.txt` - add ASan targets
- `ForgeRunner.Native/src/forge_sms_bridge.cpp` - session lifecycle
