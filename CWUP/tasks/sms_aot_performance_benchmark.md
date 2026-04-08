# SMS AOT - Performance Benchmark vs. Kotlin und C#

## Status

Open (2026-04-08)

## Motivation

SMS AOT hat gegenueber Kotlin (JVM) und C# (.NET) drei strukturelle Vorteile
gleichzeitig: keine VM, kein JIT-Warmup, kein Garbage Collector. Das sind
beweisbare Zahlen fuer einen dev.to-Artikel - kein Angeben, sondern ein
Argument fuer einen Paradigmenwechsel in der Softwareentwicklung.

"Was wenn du nie wieder eine Runtime installieren musst?"

## Messpunkte

1. **Startup-Zeit** - JVM/CLR verstecken diesen Nachteil, AOT gewinnt immer
2. **Memory Footprint** - RSS eines SMS-AOT-Prozesses vs. Hello-World in Kotlin/Android
3. **Latenz-Spitzen** - GC-Pausen bei schnellen Animationen und Echtzeit-Input
4. **Durchsatz** - CPU-bound Loop (bereits ~160x Vorteil laut bestehendem Benchmark)

## Gegner

- Kotlin/Android: JVM + GC + JIT
- C# MAUI/Xamarin: .NET Runtime + GC
- Flutter/Dart: AOT compiled, aber eigener Runtime-Layer + GC
- SMS AOT: direkt nativ, kein Overhead

## Deliverable

dev.to-Artikel mit reproduzierbaren Benchmark-Zahlen.
Benchmark-Code ins Repo unter `samples/bench_sms_vs_kotlin/`.

## Verweis

Obsidian: `ForgeStudio/SMS AOT Performance Benchmark.md`
