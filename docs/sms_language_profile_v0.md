# SMS Language Profile v0 (Draft)

Built with love, coffee, and a stubborn focus on simplicity.

## Purpose

This profile defines the next SMS language direction for:

- strong reuse in the same Forge stack
- Kotlin-familiar syntax for developer ergonomics
- SMS-native event model as the core differentiator

This is not a Kotlin clone and not a competing language branch.

## Design Principles

1. One language, multiple backends
- SMS source is canonical.
- Managed and native runtimes must share the same semantics.

2. Familiar, not identical
- Kotlin developers should feel at home quickly.
- Only adopt syntax that improves clarity and maintainability.

3. Simplicity over feature volume
- Every new feature must justify runtime and tooling complexity.
- If a feature can be deferred, defer it.

4. Event-first architecture
- `on <id>.<event>(...) { ... }` remains first-class.
- Event handling must stay simpler than mainstream UI/game scripting.

## Must-Have Scope (v0 -> v1)

### Modules and Reuse

- `import` for script/module reuse
- explicit exports
- deterministic module resolution rules

### Gradual Typing

- optional type annotations on vars and function signatures
- type checking in transpile/compile pipeline
- strict mode for CI/release, relaxed mode for rapid iteration

### Core Control Flow

- `if`, `when`, `for`, `while`, `return`
- function declarations and function calls
- predictable numeric semantics
- named arguments with strict, simple call rules

### Events

- typed event parameters (when declared)
- stable dispatch semantics across managed/native backends
- `super` event forwarding behavior defined at spec level

## Kotlin-Familiar Surface (Allowed)

- `fun`
- type annotations (`var x: Int32`)
- expression-friendly control flow where it keeps code simple
- string templates

## Explicitly Not In Scope (for now)

- complex generics
- advanced metaprogramming
- inheritance-heavy object model
- large standard library surface
- coroutine model before module/type system is stable

## Named Arguments (Normative v0)

Named arguments are part of the Kotlin-familiar surface and are allowed for function and constructor-like calls.

### Rules

1. Positional arguments may appear first.
2. After the first named argument, all following arguments must be named.
3. Unknown named argument is an error.
4. Duplicate named argument is an error.
5. Missing argument uses declared default if available; otherwise it is an error.
6. Order of named arguments is irrelevant.

### Example

```sms
var vec = Vec3D(y = 100) // x and z resolved from defaults
```

### Error Policy

- Relaxed mode: syntax and binding errors for named arguments are still hard errors.
- Strict mode: same hard errors, plus stronger type checks on bound values.

## Runtime Targets

### Short Term

- Managed interpreter (reference semantics)
- Native interpreter (performance path)

### Mid Term

- typed IR
- SMS -> C++ backend (primary)
- optional future LLVM backend via the same IR

## Quality Gates

1. Semantic parity
- shared conformance tests for managed and native runtimes

2. Performance gates
- benchmark suite for SML parse + SMS runtime hot paths
- regressions must be visible and actionable

3. Simplicity gate
- language additions require a short complexity/benefit note

## Decision Rule

If a proposal conflicts with:

- readability for app developers
- event-system clarity
- implementation simplicity

the proposal is out for v0/v1.

## Tuple Return (Normative v0)

SMS verwendet Tuple-Returns als primäres Fehlerbehandlungsmuster. Es gibt kein try-catch.

### Konvention

- Erster Wert: `Bool` (Erfolg/Fehler)
- Zweiter Wert: `String` (Nachricht, leer bei Erfolg)
- Weitere Werte: optionale Nutzdaten (`null` bei Fehler erlaubt)

### Regeln

1. Funktionen die fehlschlagen können, geben ein Tuple zurück.
2. Der Aufrufer muss das `ok`-Flag auswerten – kein silent ignore.
3. `null` als Datenwert ist nur in Tuple-Returns erlaubt (nicht als allgemeiner Typ).
4. Strict mode: unausgewertetes Tuple-Return ist ein Compiler-Fehler.

### Beispiel

```sms
fun readFile(path: String): (Bool, String, String) {
    return (false, "Datei nicht gefunden", null)
}

var (ok, msg, content) = readFile("/tmp/scene.forge")
if (!ok) {
    log.error(msg)
}
```

### Ahimsa-Prinzip

Kein Programm darf stillschweigend abstürzen. Tuple-Returns erzwingen explizites Fehlerhandling – das ist die technische Umsetzung von "tuet keinem Lebewesen Leid an" auf Code-Ebene.

## Developer Promise For Async (Normative)

Async in SMS must feel predictable from the developer perspective.

1. No hidden scope loss
- Local variables remain available after `await` in the same function scope.

2. No hidden thread model for app developers
- Developers should not need to reason about worker/main thread transitions for normal async usage.

3. Main-thread state safety
- UI and mutable runtime state are main-thread owned.
- Background work cannot directly mutate UI state.

4. Clear errors over implicit magic
- If an async usage is invalid, emit a precise compile/runtime error.
- Do not silently drop values or handlers.

5. Same semantics across backends
- Managed and native runtimes must preserve the same async scope and state behavior.
