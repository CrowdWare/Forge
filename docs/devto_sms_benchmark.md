# We Benchmarked SMS Against C++, C#, and Kotlin. Here's What Happened.

*SMS is the scripting language at the heart of Forge — an open source UI framework
built around a simple idea: what if you never had to install a runtime again?*

---

## The Question

Every new language or runtime eventually faces it:

> "But how fast is it?"

For SMS, we had been avoiding a direct answer. We knew SMS compiled to LLVM IR was
fast — but "fast" is not a number. So we sat down, designed a fair benchmark, and
let the machine decide.

Fair is the key word here. **SMS does not have an optimizer yet.** Generating LLVM IR
and handing it to clang without any `-O` flag — that is our current AOT path.
Putting SMS against C++ with `-O2` would be like sending a sprinter onto a racetrack
without shoes while the opponent wears spikes. The track would prove nothing.

So we asked a different question:

> What happens when everyone runs without optimization?

---

## The Setup

**Machine:** Apple M2  
**Workload:** Three tasks designed to resist compiler optimization even at `-O2`:

```
fib(36)             — recursive, exponential call tree
                      no compiler converts this to O(n) automatically

lcgChain(42, 2M)    — serial LCG dependency chain
                      each iteration depends on the previous → no SIMD

nestedMod(800×800)  — nested loop with modulo in the inner loop
                      modulo blocks vectorization
```

Same algorithm in every language. Same checksum required: `174148737`.
If the number doesn't match, the run is invalid.

**Compilation flags:**

| Runtime | How it was compiled |
|---|---|
| SMS → LLVM IR | `sms_compile` → `clang` (no `-O` flag) |
| C++ | `clang++ -O0` |
| C# .NET | `Release /p:Optimize=false` |
| Kotlin JVM | standard `kotlinc` + `java -jar` (JIT active) |

C++ and SMS are at exactly the same optimizer level: zero.
Kotlin and C# run with JIT — we note this explicitly in the results.

---

## The Results

**7 runs per variant. Median reported.**

| Runtime | Median (µs) | vs SMS |
|---|---:|---|
| **SMS → LLVM IR** `(no optimizer)` | **71,358** | — |
| C++ `clang -O0` | 85,466 | SMS **1.20x faster** |
| C# .NET warm `(JIT)` | 96,656 | SMS **1.35x faster** |
| Kotlin JVM warm `(JIT)` | 64,445 | Kotlin 1.11x faster |

The checksum matches across all four. The numbers are real.

**SMS without any optimizer runs 20% faster than equivalent C++ at the same
optimization level — and 35% faster than C# even with JIT active.**

Only Kotlin JVM stays ahead, and it does so with JIT assistance.

---

## Why is SMS Faster Than C++ at -O0?

This surprised us too. Here is the explanation.

When clang compiles C++ at `-O0`, it is deliberately conservative. Every variable
lives on the stack. Every intermediate value is written and re-read. The IR is
verbose by design — because at `-O0`, clang trusts the debugger more than
the programmer.

SMS skips the C++ frontend entirely. It generates LLVM IR directly from the AST,
and that IR is already lean. No frontend overhead. No stack-frame conservatism.
The result is cleaner IR that the backend can lower more efficiently — even without
optimization passes.

It is not magic. It is a shorter path.

---

## Event Dispatch: We Measured It. Then We Thought About It.

While we had the stopwatch out, we also measured event dispatch — the mechanism
behind every `on btn.clicked()` in a Forge app.

We dispatched 100,000 events to a handler (`counter = counter + 1; return counter`)
through three systems:

| Runtime | Per event (ns) | Total 100k |
|---|---:|---:|
| SMS `on bench.tick()` interpreter | 7,260 | 726 ms |
| C++ `unordered_map` dispatch | 266 | 27 ms |
| Kotlin JVM `HashMap` dispatch | 51 | 5 ms |

At first glance this looks like a problem. 27x slower than C++. 143x slower than Kotlin JVM.

Then we remembered what UI actually is.

A button click arrives roughly every 500 milliseconds if the user is fast.
At 7,260 nanoseconds per dispatch, the SMS event handler finishes **68,000 times**
before the next human input arrives.

This is not a performance gap. This is a rounding error on human time.

The real cost of a UI event is not the dispatch. It is the render, the layout pass,
the draw call. The SMS interpreter overhead disappears entirely in that noise.

We are noting it anyway — because honest benchmarks show everything, not just
the numbers that make us look good.

---

## What the Numbers Actually Mean

Let us be precise about what was measured and what was not.

**What was measured:**
- SMS AOT (LLVM IR, no optimizer) vs C++ -O0 vs C#/Kotlin JIT
- A compute workload on Apple M2
- A synthetic event dispatch loop

**What was not measured:**
- C++ with `-O2` — it would win by a large margin, and that is fine
- SMS with an optimizer — that does not exist yet
- Real UI rendering, startup time, memory footprint

The honest picture:

```
today:
  SMS (no optimizer)  ████████████  71 ms
  C++ (-O0)           ██████████████  85 ms
  C# (JIT)            ████████████████  97 ms
  Kotlin (JIT)        ███████████  64 ms

with C++ -O2:
  C++ (-O2)           ███  ~8-12 ms   ← would win

SMS with optimizer (not yet):
  SMS (-O2)           ???             ← where this is going
```

We are not claiming SMS is faster than C++. We are saying:
**SMS generates better unoptimized code than C++ generates unoptimized code.**
That is a meaningful statement about the quality of the compiler backend.
When an optimizer arrives, it starts from a better baseline.

---

## The Next Step: JIT

There is one scenario where the event dispatch latency does matter:
high-frequency programmatic events — animations, simulation loops, reactive data
bindings firing hundreds of times per second.

For that, SMS already has every building block for JIT:

| Piece | Status |
|---|---|
| Interpreter | ✅ `sms_native_session_invoke()` |
| LLVM IR codegen | ✅ `sms_native_codegen_llvm_ir()` |
| clang as backend | ✅ used by `sms_compile` |
| `dlopen` / `dlsym` | ✅ standard POSIX |

The missing piece: after a handler fires N times, compile it to a shared library,
`dlopen` it, swap the function pointer. ~200–300 lines of C++.

After warmup, a JIT-compiled SMS handler would reach native C++ dispatch speeds.
Kotlin JVM would no longer have a structural advantage — it would be an equal.

**We are looking for someone to build this.**

If you are comfortable with C++17 and `dlopen`, the architecture is documented,
the benchmark harness is ready, and your name goes into the release notes and
into this article.

→ [GitHub Issue: SMS JIT Dispatcher](https://github.com/CrowdWare/sms-cpp/issues)

---

## Why Forge

Forge is a UI framework built on two ideas:

**SML** — a declarative markup language for UI layout.  
**SMS** — an event-driven scripting language that compiles to LLVM IR.

No JVM. No CLR. No garbage collector. No runtime to install.
`on btn.clicked()` is not a callback registered in a framework lifecycle.
It is a first-class language construct that compiles to native code.

We benchmarked it because we want to know the truth about where we stand.
The truth is: we are faster than C++ at the same level, faster than C# JIT,
and one optimizer away from being a serious contender against the full stack.

That is not a marketing claim. That is a measured number with a checksum.

---

*Benchmark code: `samples/bench_mac/` in the Forge repository.*  
*Run it yourself: `bash samples/bench_mac/run_bench.sh`*

*Forge is open source. Contributions welcome.*  
*Sat Nam. 🌱*
