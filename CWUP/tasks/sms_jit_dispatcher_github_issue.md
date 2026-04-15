# [Contributor Wanted] SMS JIT Dispatcher — close the gap with Kotlin JVM

## Background

We just ran a cross-language benchmark on Apple M2.
Workload: recursive Fibonacci + LCG chain + nested modulo loop.
All variants produce the same checksum (`174148737`). Verified.

**Compute — same optimizer level, no tricks:**

| Runtime | µs | |
|---|---:|---|
| **SMS → LLVM IR** (no optimizer) | **71,358** | 🏆 |
| C++ `clang -O0` | 85,466 | SMS 1.20x faster |
| C# .NET warm (JIT) | 96,656 | SMS 1.35x faster |
| Kotlin JVM warm (JIT) | 64,445 | JIT active |

SMS without any optimizer already beats C++ at the same optimization level.
It beats C# JIT by 35%. Only Kotlin stays ahead — because it JITs.

**Event dispatch — where the gap lives:**

| Runtime | ns / event |
|---|---:|
| SMS `on bench.tick()` interpreter | 7,260 |
| C++ `unordered_map` | 266 |
| Kotlin JVM `HashMap` | 51 |

Every `on id.clicked()` in a Forge app goes through the interpreter.
That's 7 µs per user interaction. With a JIT dispatcher it drops to ~266 ns.

## What needs to be built

A **threshold-based JIT dispatcher** inside the SMS session runtime.

The building blocks already exist:

| Piece | Already there |
|---|---|
| Interpreter | `sms_native_session_invoke()` |
| LLVM IR codegen | `sms_native_codegen_llvm_ir()` |
| clang as backend | used by `sms_compile` |
| dlopen / dlsym | standard POSIX |

The missing piece: after a handler fires N times (threshold ~50),
compile it to a shared library at runtime, `dlopen` it, swap the
interpreter call for a native function pointer. ~200–300 lines of C++.

## Scope

- Hit counter per event handler in session state
- Compile hot handler: `sms_native_codegen_llvm_ir` → `clang -shared -O2` → `.dylib`/`.so`
- `dlopen`/`dlsym` swap in dispatcher  
- Silent fallback to interpreter on compile failure
- Unit test: 100 dispatches → verify native path taken
- Benchmark: `samples/bench_mac/run_bench.sh` Part 2 before/after

## Skills

C++17 · POSIX `dlopen`/`dlsym` · comfortable reading LLVM IR

## Files to touch

```
sms-cpp/src/sms_impl_runtime.cpp   ← dispatcher lives here
sms-cpp/src/sms_impl_codegen.cpp   ← expose sms_jit_entry symbol
sms-cpp/include/sms_native.h       ← API stays stable
sms-cpp/tests/                     ← add JIT dispatch test
```

Reference: `sms-cpp/cli/main.cpp` shows the full codegen → clang pipeline.

## Reward

Your name in the release notes and in the benchmark article on dev.to.
You'll have added native-speed event dispatch to an open-source UI framework
that already outperforms C# JIT — without any optimizer.

**The seed is planted. Come help it grow. 🌱**
