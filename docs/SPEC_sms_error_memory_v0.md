# SMS Error Handling & Memory Model v0

## Leitprinzip

> "Tuet keinem Lebewesen Leid an."

Auf Code-Ebene bedeutet das:
- Kein stiller Absturz
- Kein eingefrorenes UI durch GC-Pausen
- Kein Speicherleck
- Keine Überraschungen zur Laufzeit

---

## 1) Fehlerbehandlung

### Compile-Zeit Fehler (streng wie Rust)

**Alles was der Compiler wissen kann, ist ein Compiler Error.**

Der Compiler ist streng – lieber früh scheitern als zur Laufzeit überraschen.

Compile Errors enthalten immer:
- Fehlertyp
- Dateiname, Zeile, Spalte
- Betroffene Codezeile als Kontext

```
[SMS Compiler Error] type_mismatch
  file: main.sms, line 12, col 5
  expected: Int32
  got: String
  > count = "hallo"
```

**Beispiele für Compile Errors:**
- Typ-Konflikt bei Neuzuweisung
- Unbekannte Variable
- Falscher Tuple-Return Typ
- Unbekanntes Named Argument
- Duplicate Named Argument

### Typ ist fix nach erster Zuweisung

```sms
var count = 0        // inferiert: Int32 – für immer
count = "hallo"      // COMPILER ERROR: type_mismatch
```

Kein dynamisches Umentscheiden zur Laufzeit. Kein JavaScript. Kein Kompost.

### Source Maps (Pflicht)

Jeder AST-Node speichert Zeile + Spalte aus dem Quellcode.
Source Maps müssen von Anfang an im Compiler vorhanden sein – nachrüsten ist schmerzhaft.

### Runtime Fehler (abgefangen, kein Crash)

Was erst zur Laufzeit bekannt ist, wird als Runtime Error gemeldet –
aber die Host-App crasht **niemals**.

SMS verhält sich wie ein Browser: JavaScript-Fehler auf einer Seite crashen nicht den Browser.
SMS Runtime Errors beenden den SMS-Thread sauber – nicht die Host-App.

```
[SMS Runtime Error] division_by_zero
  file: math.sms, line 34, col 12
  > result = a / b
  SMS thread terminated. App continues.
```

**Beispiele für Runtime Errors:**
- Division durch Null
- Index out of bounds
- Null-Zugriff in Tuple Return

---

## 2) Memory Model

### Kein Garbage Collector. Kein manuelles Free.

SMS hat bewusst kein Memory Management spezifiziert.
Das Ergebnis: **LLVM's RAII unter der Haube** – automatisch, deterministisch, ohne GC-Pausen.

### Warum das besonders ist

| Sprache | Memory Model | GC-Pause möglich |
|---------|-------------|-----------------|
| Python | Garbage Collector | ✅ ja |
| Kotlin | Garbage Collector | ✅ ja |
| Swift | ARC (Reference Counting) | ❌ nein |
| Rust | RAII + Ownership | ❌ nein (aber komplex) |
| C++ | RAII (manuelles Free möglich) | ❌ nein (aber gefährlich) |
| **SMS** | **RAII via LLVM, nicht spezifiziert** | **❌ nein** |

Rust hat dasselbe Ziel – aber durch ein komplexes Regelwerk erzwungen (Borrow Checker, Lifetimes).
SMS erreicht es durch Einfachheit: LLVM macht es richtig darunter, SMS bleibt sauber oben.

### Stack vs Heap

- **Stack**: Primitive Typen, lokale Variablen → automatisch freigegeben wenn Scope endet
- **Heap**: Strings, dynamische Daten → Reference Counting via LLVM ARC

### Ahimsa im Memory Model

Kein GC bedeutet:
- Keine unvorhersehbaren Pausen
- Kein eingefrorenes UI
- Deterministisches Verhalten

Das ist die technische Umsetzung von "tuet keinem Lebewesen Leid an" auf Memory-Ebene.

---

## Zusammenfassung

| Thema | Entscheidung |
|-------|-------------|
| Compile Errors | Streng, mit Source Maps, Zeile + Spalte |
| Runtime Errors | Abgefangen, SMS-Thread endet, App läuft weiter |
| Typ nach Zuweisung | Fix, kein dynamischer Typ-Wechsel |
| Garbage Collector | Keiner |
| Manuelles Free | Keines |
| Memory Management | RAII via LLVM, automatisch |
