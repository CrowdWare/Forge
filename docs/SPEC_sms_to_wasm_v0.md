# Spezifikation: sms-to-wasm v0

## Ziel

SMS-Quellcode zu WebAssembly kompilieren.
Zielplattformen: Browser, IPFS, CrowdBooks.

## Ansatz: SMS → LLVM IR → WASM

SMS wird bereits zu LLVM IR kompiliert (`sms-to-llvm`).
LLVM kann WASM als Compile-Target ausgeben – dieser Weg wird genutzt.

**Kein eigener WASM-Codegen notwendig.**

```
SMS Source
    ↓  (sms-to-llvm, bereits vorhanden)
LLVM IR
    ↓  (llc / clang --target=wasm32, LLVM-seitig)
WASM Binary (.wasm)
```

## Qualitätsanspruch

> "Sauberer als JavaScript."

Das ist der Mindestanspruch – und er ist leicht zu erfüllen:
- Typisiert (keine impliziten Typumwandlungen)
- Deterministisch (kein `undefined`, kein `null` außer in Tuple Returns)
- Kein Laufzeit-Chaos (kein try-catch nötig, Tuple Return stattdessen)

## Scope v0

### In Scope

- SMS Core-Features kompilieren zu WASM:
  - Variablen + Type Inference
  - Funktionen
  - Control Flow (`if`, `when`, `for`, `while`)
  - Tuple Return (Fehlerbehandlung ohne Exceptions)
  - String Templates
- Standalone CLI: `sms-to-wasm input.sms -o output.wasm`
- WASM läuft im Browser (via `<script type="module">`)
- WASM läuft in Node.js (für Tests)

### Nicht in Scope (v0)

- Event-System (`on`-Keyword) – das ist UI, nicht WASM-Kern
- DOM-Zugriff direkt aus SMS
- Async/Await
- Komplexe Generics

## LLVM WASM Target Setup

```bash
# LLVM mit WASM-Support (einmalig)
apt install llvm clang lld

# Kompilierung SMS → LLVM IR → WASM
sms-to-llvm input.sms -o output.ll
llc -march=wasm32 -filetype=obj output.ll -o output.o
wasm-ld output.o -o output.wasm --no-entry --export-all
```

## Abnahme-Kriterium

Dieses SMS-Programm:

```sms
fun add(a: Int32, b: Int32): Int32 {
    return a + b
}

fun safeDivide(a: Int32, b: Int32): (Bool, String, Int32) {
    if (b == 0) {
        return (false, "Division durch Null", 0)
    }
    return (true, "ok", a / b)
}
```

kompiliert zu validen WASM und läuft korrekt im Browser und in Node.js.

## Zusammenhang mit anderen Repos

| Repo | Rolle |
|------|-------|
| `sml-parser` | Parst SML/SMS Quellcode zu AST |
| `sms-to-llvm` | SMS AST → LLVM IR (Voraussetzung) |
| `sms-to-wasm` | LLVM IR → WASM (dieser Task) |
| `crowdbooks` | Nutzt sms-to-wasm für Browser/IPFS |

## Nächste Schritte

- [ ] LLVM WASM Target lokal verifizieren
- [ ] `sms-to-llvm` Output als Eingabe für WASM-Pipeline testen
- [ ] CLI-Wrapper bauen
- [ ] Smoke-Test im Browser
- [ ] In CrowdBooks integrieren
