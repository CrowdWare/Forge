# SMS JIT Dispatcher — Community Contribution

## Status
Open — seeking contributor

## The Story

We benchmarked SMS compiled to LLVM IR against C++, C# and Kotlin JVM on Apple M2.
The workload was designed to be optimizer-resistant (recursive fib, serial LCG chain,
nested loop with modulo) so no compiler gets an unfair advantage.

**Results (Apple M2, median of 7 runs):**

| Runtime | µs | Notes |
|---|---:|---|
| SMS → LLVM IR (no optimizer) | 71,358 | our baseline |
| C++ clang -O0 | 85,466 | SMS 1.20x faster |
| C# .NET warm | 96,656 | SMS 1.35x faster |
| Kotlin JVM warm | 64,445 | JIT active |

SMS without any optimizer already beats C++ at the same optimizer level by 20%,
and beats C# JIT by 35%. Only Kotlin JVM edges ahead — because it JITs.

**Event dispatch results (100k events, 1k warmup):**

| Runtime | ns / event |
|---|---:|
| SMS interpreter (`on bench.tick()`) | 7,260 |
| C++ `unordered_map` dispatch | 266 |
| Kotlin JVM `HashMap` dispatch | 51 |

The interpreter dispatch is the gap. Every `on id.clicked()` in a Forge app
goes through the interpreter. A JIT dispatcher would close this to near-native.

## The Opportunity

SMS already has every building block needed for JIT:

| Building block | Location |
|---|---|
| Interpreter | `sms_native_session_invoke()` in `sms-cpp` |
| LLVM IR codegen | `sms_native_codegen_llvm_ir()` in `sms-cpp` |
| clang as backend | already required by `sms_compile` |
| `dlopen` / `dlsym` | standard POSIX |

What is missing: a **hit counter per event handler** + **threshold check** +
**compile-and-swap** in the dispatcher. Roughly 200–300 lines of C++.

## Implementation Sketch

```cpp
// In the SMS session/dispatcher:

struct HandlerEntry {
    std::string   sms_source;
    int           hit_count   = 0;
    void*         native_fn   = nullptr;   // nullptr = still interpreted
    void*         dl_handle   = nullptr;
};

static constexpr int kJitThreshold = 50;   // tune to taste

int64_t dispatch(Session& s, const std::string& key, ...) {
    auto& h = s.handlers[key];
    h.hit_count++;

    if (h.native_fn) {
        // hot path — compiled already
        return reinterpret_cast<int64_t(*)()>(h.native_fn)();
    }

    if (h.hit_count == kJitThreshold) {
        // compile in background (or inline for simplicity)
        char ir[256*1024] = {};
        char err[512]     = {};
        if (sms_native_codegen_llvm_ir(h.sms_source.c_str(),
                                        ir, sizeof(ir), err, sizeof(err)) == 0) {
            std::string tmp = write_temp_ll(ir);
            std::string so  = compile_to_so(tmp);   // clang -O2 -shared
            h.dl_handle  = dlopen(so.c_str(), RTLD_NOW);
            h.native_fn  = dlsym(h.dl_handle, "sms_jit_entry");
        }
    }

    // cold path — interpret as usual
    return interpret(s, key, ...);
}
```

The `sms_jit_entry` symbol is the function the codegen must expose —
a small addition to `sms_impl_codegen.cpp`.

## Scope

- [ ] Hit counter per handler in `sms_native_session_*` structs
- [ ] Threshold-triggered compile: `sms_native_codegen_llvm_ir` → temp `.ll` → `clang -shared -O2` → `.dylib`/`.so`
- [ ] `dlopen`/`dlsym` swap in dispatcher
- [ ] Fallback to interpreter on compile failure (silent, no crash)
- [ ] Unit test: dispatch 100 × same handler → assert native path taken
- [ ] Benchmark: reproduce `samples/bench_mac/run_bench.sh` Part 2 before/after

## Out of Scope

- Windows support (optional follow-up)
- Background thread compilation (optional follow-up)
- Tiered optimization levels

## Skills needed

C++17, basic POSIX (`dlopen`/`dlsym`), comfort reading LLVM IR output.

## Reward

Your name in the release notes and the benchmark article on dev.to.
You will have contributed native-speed event dispatch to an open source
UI framework that already outperforms C# JIT without any optimizer.

## References

- Benchmark code: `samples/bench_mac/`
- SMS codegen:    `sms-cpp/src/sms_impl_codegen.cpp`
- SMS API header: `sms-cpp/include/sms_native.h`
- CLI compiler:   `sms-cpp/cli/main.cpp` (reference for the codegen → clang pipeline)
