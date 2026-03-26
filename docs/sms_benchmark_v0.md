# SMS Benchmark v0

## 1) Aktuelle Messung: SMS Native vs C# vs GDScript

### Protokoll

- 5 Warmup-Runs (JIT/Cache-Effekte eliminiert)
- 20 gemessene Runs
- Median + p95 (kein Cherry-picking)
- Gleiche Maschine, gleicher Workload

### Ergebnisse

| Runtime   | Median (µs) | p95 (µs)  |
|-----------|-------------|-----------|
| SMS Native | 3.466      | 3.502     |
| C#         | 4.382      | 4.436     |
| GDScript   | 258.220    | 261.709   |

### Speedups

| Vergleich                  | Median  | p95     |
|----------------------------|---------|---------|
| SMS Native vs C#           | 1.26x   | 1.27x   |
| SMS Native vs GDScript     | 74.50x  | 74.73x  |

### Einordnung

Das sind keine Tweet-sized Benchmark-Claims. Das sind gemessene, wiederholbare Daten vom selben Workload.

> **Hinweis**: Workload-Details noch zu dokumentieren. Wichtig für spätere Vergleichbarkeit.

---

## 2) Geplanter Benchmark: SMS/Forge vs Kotlin/Compose

### Ziel

Direkter Vergleich von Forge 4D (SMS Native) mit Kotlin + Compose (JVM) anhand einer identischen App.

### Hintergrund

Forge 4D hat seine Wurzeln im Noco Designer, der ursprünglich mit Kotlin/Compose gebaut wurde. Der Wechsel zu SMS/Forge war eine bewusste Entscheidung. Dieser Benchmark soll die Performance-Dimension dieser Entscheidung messbar machen.

### Warum nicht ForgePoser als Vergleichs-App?

**Forge kann mehr als Compose.**

ForgePoser nutzt 3D-Controls und Forge-spezifische UI-Elemente, die in Kotlin/Compose nicht existieren. Ein direkter Nachbau ist nicht möglich.

> "Wir wollten Forge mit Compose vergleichen, aber da vergleichen wir Apple mit Kompost."
> — Art, März 2026

### Vergleichs-App (noch zu bauen)

Eine abgespeckte App, die beide Systeme vollständig abbilden können:

- Liste mit Items (Scroll-Performance)
- Interaktive Buttons (Event-Reaktivität)
- 2D Canvas-Element (Render-Performance)

Gleiche UI, gleiche Logik, gleiche Daten – dann messen.

### Metriken

- **Cold Start** (Startup-Zeit)
- **Frame-Zeit** (UI-Reaktivität, Scroll)
- **Memory-Footprint** (RSS bei idle)
- **CPU unter Last** (Render-Loop)

### Feature-Statement (über den Benchmark hinaus)

Der Benchmark endet dort, wo Compose aufhört. Forge geht weiter:

| Feature          | Forge 4D | Kotlin/Compose |
|------------------|----------|----------------|
| 3D Controls      | ✅       | ❌             |
| Native Performance| ✅      | ❌ (JVM)       |
| SMS Event-First  | ✅       | ❌             |
| Eigene Controls  | ✅       | eingeschränkt  |
| Plattform-unabhängig | ✅   | Android-first  |

---

## Status

- [x] SMS vs C# vs GDScript gemessen
- [ ] Vergleichs-App definieren
- [ ] Vergleichs-App in Kotlin/Compose bauen
- [ ] Vergleichs-App in SMS/Forge bauen
- [ ] Benchmark durchführen und dokumentieren
