# SMS vs C++ vs Kotlin JVM — Mac Benchmark
**Date:** 2026-04-15
**Machine:** Apple M2
**Runs:** 7 per variant, median reported

## Part 1 — Compute Benchmark
Workload: `fib(36)` + `lcgChain(42, 2_000_000)` + `nestedMod(800×800)`

> **Fairness:** SMS compiles to LLVM IR via `sms_compile` (no optimizer).
> C++ compiled with `clang -O0` (same level). Kotlin/C# have JIT (noted).

| Runtime | Median (µs) | vs SMS |
|---|---:|---:|
| SMS → LLVM IR `(no -O)` | 71358.0 | — |
| C++ `clang -O0` | 85465.7 | SMS 1.20x faster |
| C# .NET cold `(JIT)` | 97378.2 | SMS 1.36x faster |
| C# .NET warm `(JIT)` | 96655.5 | SMS 1.35x faster |
| Kotlin JVM cold `(JIT)` | 64893.4 | 1.10x slower |
| Kotlin JVM warm `(JIT)` | 64445.2 | 1.11x slower |

Checksum (all must match): `174148737`

## Part 2 — Event Dispatch Benchmark
100 000 events · 1 000 warmup · handler: `counter = counter + 1`

| Runtime | Per event (ns) | Total 100k (µs) | vs SMS |
|---|---:|---:|---:|
| SMS `on bench.tick()` interpreter | 7260.2 | 726024.2 | — |
| C++ `unordered_map` dispatch | 266.4 | 26637.8 | 27.3x faster |
| Kotlin JVM `HashMap` dispatch | 50.7 | 5071.1 | 143.2x faster |

