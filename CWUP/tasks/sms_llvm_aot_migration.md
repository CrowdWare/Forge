# SMS: LLVM IR AOT Migration

## Status

Implemented (current scope, 2026-03-24).

## Context

The SMS benchmark (see `SMSCore.Native/benchmark/`) shows that the SMS interpreter
is ~160x slower than compiled native code for a CPU-bound loop (Fibonacci, 1M iterations).

SMS → LLVM IR codegen is already implemented in Phase 1 (`sms_native_codegen_llvm_ir`).
It generates correct `.ll` files and produces identical results to the interpreter.

## Goal

Migrate SMS execution to use LLVM IR AOT compilation for local execution,
while keeping the interpreter for remote/HTTP content.

## Execution Strategy

| Source                  | Runtime          | Rationale                                      |
|-------------------------|------------------|------------------------------------------------|
| `http://` / `https://`  | Interpreter      | Untrusted, sandboxed, no compile step possible |
| `file://` / local path  | LLVM AOT (.so)   | Trusted, compile once, load fast               |

## Phase 2 — AOT Pipeline (local)

1. `sms_native_codegen_llvm_ir(source)` → `.ll` string
2. Compile via `llvm::Module` + LLVM JIT (in-process, no `clang` subprocess)
   OR write `.ll` + `clang -O2 -shared` → `.so` / `.dylib`
3. Load `.so` via `dlopen`, resolve exported entry points
4. Call compiled functions directly — no interpreter overhead

Compiled artifacts are cached by source hash:
- Cache key: `sha256(sms_source)`
- Cache location: `~/.cache/forge-runner/sms_aot/{hash}.so`
- On load: check cache first, compile only if missing or source changed

### Implemented Notes

- `sms_native_aot_invoke(...)` is implemented in `SMSCore.Native`.
- Local AOT path uses `clang -O2 -shared` to build a cached shared library and invokes `main` via `dlopen`/`dlsym` (or platform equivalent).
- Cache key is `sha256(source)`.
- AOT cache directory defaults to `~/.cache/forge-runner/sms_aot` (override: `SMS_NATIVE_AOT_CACHE_DIR`).
- ForgeRunner dispatch now prefers AOT for local/trusted sources and keeps interpreter fallback on any AOT failure.
- Remote HTTP/HTTPS app sources are forced to interpreter path.
- Added compiler/AOT tests in `SMSCore.Native/tests/compiler_codegen_tests.cpp` and CTest registration in `SMSCore.Native/CMakeLists.txt`.

## Phase 4 — ForgeRunner.Native Integration

`forge_runner_main.cpp` currently calls `sms_native_session_invoke` for all events.
Add a dispatch layer:

```cpp
if (is_local_source && aot_cache.has(source_hash)) {
    aot_cache.invoke(source_hash, target_id, event_name, args);
} else {
    sms_native_session_invoke(session, target_id, event_name, args, ...);
}
```

### Implemented Integration Notes

- `ForgeRunner.Native/src/forge_sms_bridge.cpp` now tracks session metadata (local/remote, handler presence, AOT status).
- For local sessions without SMS event handlers, bridge attempts AOT once and:
  - keeps AOT active on success,
  - falls back to interpreter on failure with warning.

## Out of Scope

- WebAssembly target (separate story)
- Android AOT (same Linux pipeline, defer until macOS is stable)
- String / Array / Object codegen (Phase 1 only handles Int, Bool)

## Release Notes (English)

- Added LLVM AOT execution path for trusted local SMS sources in ForgeRunner Native.
- Added shared-library based AOT invocation (`dlopen`/`dlsym`) with `sha256(source)` cache.
- Preserved interpreter fallback for all AOT failures and for remote HTTP/HTTPS sources.
- Added compiler/codegen/AOT tests, including cache-reuse and shared-library artifact coverage.
