# Tasks: Kristallisiere SMS/SML Tools als eigenständige Repos

Vorbild: `sml.go` wurde bereits erfolgreich als eigenständiges Repo kristallisiert.
Diese Tasks folgen demselben Muster.

---

## [TASK-1] Kristallisiere `sml-parser` als eigenständiges Repo

**Status:** `sml-parser` ist noch direkt in Forge integriert.
**Ziel:** Eigenständiges Repo, Forge nutzt es als externe Dependency.

### Schritte

1. Alle SML-Parser-relevanten Dateien in Forge identifizieren
2. Abhängigkeiten zum Rest von Forge dokumentieren
3. Neues Repo anlegen: `sml-parser`
4. Code verschieben, interne Abhängigkeiten lösen
5. Öffentliche API definieren (was Forge wirklich braucht)
6. Forge auf das neue Repo als Dependency umstellen
7. Tests sicherstellen dass Forge weiterhin funktioniert

### Abnahme-Kriterium

Forge kompiliert und läuft identisch wie vorher, aber `sml-parser` liegt in eigenem Repo.

---

## [TASK-2] Kristallisiere `sms-to-llvm` als eigenständiges Repo

**Status:** `sms-to-llvm` ist fest verdrahtet in Forge CLI.
**Ziel:** Eigenständiges Repo, Forge CLI nutzt es als Dependency.

### Kontext

Der SMS-to-LLVM-IR Compiler ist das Herzstück der nativen Performance.
Als eigenständiges Tool kann er unabhängig versioniert, getestet und von anderen Projekten genutzt werden.

### Schritte

1. Alle sms-to-llvm relevanten Dateien in Forge CLI identifizieren
2. Einstiegspunkt und öffentliche API definieren
3. Neues Repo anlegen: `sms-to-llvm`
4. Code verschieben, Abhängigkeiten zu Forge CLI lösen
5. Standalone CLI-Interface bauen: `sms-to-llvm input.sms -o output.ll`
6. Forge CLI auf neues Repo als Dependency umstellen
7. Bestehende Benchmarks gegen neue Struktur laufen lassen

### Abnahme-Kriterium

`sms-to-llvm` läuft als standalone Tool.
Forge CLI liefert identische Benchmark-Ergebnisse wie vorher.

---

## [TASK-3] Erstelle neues Repo `sms-to-wasm`

**Status:** Existiert noch nicht.
**Ziel:** SMS zu WebAssembly Compiler, eigenständiges Repo von Anfang an.

### Kontext

`sms-to-wasm` wird gebraucht für:
- **CrowdBooks** (IPFS, Browser-fähig)
- Zukünftige Web-Targets von Forge 4D

`sms-to-llvm` dient als Referenz-Implementierung für Semantik und Verhalten.
Beide Backends teilen dieselbe SMS-Source – das ist das "One language, multiple backends" Prinzip.

### Schritte

1. Repo anlegen: `sms-to-wasm`
2. Grundstruktur nach Vorbild von `sms-to-llvm`
3. SMS AST als gemeinsame Basis definieren (ggf. aus `sms-to-llvm` extrahieren)
4. WASM-Codegen für SMS Core-Features implementieren:
   - Variablen + Typen
   - Funktionen
   - Control Flow (if, when, for)
   - Tuple Return
5. Standalone CLI: `sms-to-wasm input.sms -o output.wasm`
6. Einfache Smoke-Tests

### Abnahme-Kriterium

Ein einfaches SMS-Programm (Funktion, Loop, Tuple Return) kompiliert zu validen WASM und läuft im Browser.

---

## Reihenfolge (Empfehlung)

```
TASK-1 (sml-parser)  →  TASK-2 (sms-to-llvm)  →  TASK-3 (sms-to-wasm)
```

TASK-1 zuerst, weil sml-parser eine Abhängigkeit von sms-to-llvm ist.
TASK-3 kann parallel starten sobald der SMS AST aus TASK-2 stabil ist.

---

## Repo-Übersicht nach Abschluss

| Repo | Sprache | Zweck |
|------|---------|-------|
| `sml.go` | Go | SML Parser (bereits draußen) |
| `sml-parser` | C++ / Go | SML Parser als Library |
| `sms-to-llvm` | C++ | SMS → LLVM IR (native Performance) |
| `sms-to-wasm` | C++ / Go | SMS → WebAssembly (Browser, IPFS) |
| `forge` | C++ | Forge 4D Framework (nutzt obige als Dependencies) |
