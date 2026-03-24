# SMS: LLVM IR AOT Migration

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

## Out of Scope

- WebAssembly target (separate story)
- Android AOT (same Linux pipeline, defer until macOS is stable)
- String / Array / Object codegen (Phase 1 only handles Int, Bool)
