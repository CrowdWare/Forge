# SMS vs C++ vs Kotlin JVM vs Kotlin Native — Mac Benchmark
**Date:** 2026-04-16
**Machine:** Apple M2
**Runs:** 7 per variant, median reported

## Part 1 — Compute Benchmark
Workload: `fib(36)` + `lcgChain(42, 2_000_000)` + `nestedMod(800×800)`

> **Fairness:** SMS compiles to LLVM IR via `sms_compile` (no optimizer).
> C++ compiled with `clang -O0` (same level). Kotlin JVM / C# have JIT (noted).
> Kotlin Native compiled AOT without `-opt` — closest to SMS/C++ conditions.

| Runtime | Median (µs) | vs SMS |
|---|---:|---:|
| SMS → LLVM IR `(no -O)` | 70967.0 | — |
| C++ `clang -O0` | 84764.0 | SMS 1.19x faster |
| C# .NET cold `(JIT)` | 96881.8 | SMS 1.37x faster |
| C# .NET warm `(JIT)` | 97083.8 | SMS 1.37x faster |
| Kotlin JVM cold `(JIT)` | 107325.9 | SMS 1.51x faster |
| Kotlin JVM warm `(JIT)` | 64326.6 | 1.10x slower |
| Kotlin Native `(AOT, no -opt)` | 49933.4 | 1.42x slower |

Checksum (all must match): `174148737`

## Part 2 — Event Dispatch Benchmark
100 000 events · 1 000 warmup · handler: `counter = counter + 1`

| Runtime | Per event (ns) | Total 100k (µs) | vs SMS |
|---|---:|---:|---:|
| SMS `on bench.tick()` interpreter | 7219.0 | 721902.2 | — |
| C++ `unordered_map` dispatch | 393.3 | 39331.1 | 18.4x faster |
| Kotlin JVM `HashMap` dispatch | 47.7 | 4766.9 | 151.3x faster |
| Kotlin Native `HashMap` (AOT) | 8.4 | 843.2 | 859.4x faster |

