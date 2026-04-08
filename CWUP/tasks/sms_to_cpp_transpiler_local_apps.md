# SMS → C++ Transpiler For Local Apps

## Goal
Add an optional transpiler that converts SMS scripts directly to C++ for apps
that run locally and are not deployed over HTTP - eliminating interpreter overhead
in production-local builds while keeping the authoring model unchanged.

## Why
- Remove interpreter overhead in production-local builds.
- Keep authoring model unchanged: users continue writing SMS only.
- The interpreted path stays active for HTTP/deployed scenarios (hot-reload, no rebuild).

## Architecture: SMS AST → C++ (direct, no IL)

The SMS AST produced by the existing `SMSCore.Native` parser is the natural
intermediate representation. A second backend consuming the same AST is all
that is needed - a separate IL format would add spec maintenance, versioning
overhead, and a second transformation pass with no immediate benefit.

```
SMS source
    ↓  (existing SMSCore.Native lexer + parser - unchanged)
SMS AST
    ↓  (new: SmsCodegenCpp)
Generated .cpp + .h
    ↓  (CMake / ninja)
Native binary - no interpreter in hot path
```

If a future additional backend is ever needed, it adds a second consumer of
the same SMS AST - not a consumer of a custom IL.

## Why Not IL

- `SMSCore.Native` is already native C++. Interpreter overhead is only relevant
  for tight loops or heavy computation, which UI event handlers are not.
- **Evaluate first**: before implementing the transpiler, profile a
  representative local app to confirm the interpreter is actually the bottleneck.
  If it isn't, the interpreted path is the right answer and this task is deferred.
- An IL layer is justified only when two or more concrete backends exist
  simultaneously. With one target (C++) the SMS AST is the IR.

## Scope
- Implement `SmsCodegenCpp`: an AST walker that emits a `.cpp` + `.h` pair per
  SMS script.
- Integrate into local build flow only (`--sms-transpile=true` flag).
- Keep the standard interpreted SMS path for HTTP/deployed scenarios (no change).
- Document which SMS constructs are supported; unsupported constructs emit a
  clear compile-time diagnostic and abort the build.

## Non-Goals
- No IL format, no IL spec, no IL versioning.
- No additional language backends (Compose/JVM path is deprecated).
- No JIT.
- No remote/HTTP deployment transpilation.
- No changes to the SMS authoring language.

## Supported Subset (v1)
The first version targets the constructs actually used in practice:

| Construct | Status |
|---|---|
| `var`, assignment, arithmetic | ✅ |
| `if` / `else` | ✅ |
| `while`, `for … in` | ✅ |
| `fun` (top-level functions) | ✅ |
| `on` (event handler registration) | ✅ |
| Native function calls (`ui.*`, `os.*`, `log.*`) | ✅ via generated glue |
| Closures / first-class functions | - (not in SMS language) |
| Dynamic `ui.getObject()` with runtime id | ⚠️ supported with warning |
| `Array`, `Dictionary` literals | ✅ |
| Recursive functions | ✅ with depth guard |
| `data class` | ✅ → generated C++ struct |
| `null` checks | ✅ |
| String concatenation (`+`) | ✅ |

---

## Phase 0: Profiling Gate

Before any code is written, profile ForgePoser on a representative workload:

```bash
# macOS instruments / perf
./run.sh poser --profile-sms
```

- If SMS interpreter < 5 % of frame time → defer this task, mark blocked.
- If ≥ 5 % → proceed to Phase 1.

---

## Phase 1: Design Decisions (Pre-Implementation)

### 1.1  Codegen API - AST stays internal

The SMS AST types (`MemberAccessExpr`, `AssignStmt`, etc.) are currently
internal to `sms_native.cpp`. **Do not expose them in a header.**

Instead, add a single new exported function to `sms_native.h`:

```c
// New export in sms_native.h
SMS_EXPORT int sms_native_codegen_cpp(
    const char* source,          // SMS source text
    char* out_cpp, int out_cpp_cap,  // generated .cpp content
    char* out_h,   int out_h_cap,   // generated .h content
    char* error,   int error_cap);
```

`SmsCodegenCpp` is implemented inside `SMSCore.Native/src/sms_codegen_cpp.cpp`
and `#include`d or linked into `sms_native.cpp` - the AST types remain in scope
without header exposure.

### 1.2  Value type in generated code: reuse `SmsValue`

The generated C++ uses a thin runtime type defined in `sms_native_glue.h`.
For V1 this is a tagged union identical in shape to the internal `Value` struct:

```cpp
// sms_native_glue.h  (hand-written, not generated)
struct SmsValue {
    enum class Kind { Int, Bool, String, Array, Object, Null };
    Kind kind = Kind::Null;
    std::int64_t    int_value   = 0;
    bool            bool_value  = false;
    std::string     string_value;
    std::shared_ptr<std::vector<SmsValue>>                     array;
    std::string                                                 class_name;
    std::shared_ptr<std::unordered_map<std::string, SmsValue>> object_fields;

    bool truthy() const;

    // Convenience constructors
    static SmsValue Int(std::int64_t v);
    static SmsValue Bool(bool v);
    static SmsValue String(std::string v);
    static SmsValue Null();
    static SmsValue UiRef(std::string id);  // class_name == "__ui_ref"

    // Arithmetic / comparison operators for generated expressions
    SmsValue operator+(const SmsValue&) const;
    SmsValue operator-(const SmsValue&) const;
    SmsValue operator*(const SmsValue&) const;
    SmsValue operator/(const SmsValue&) const;
    bool operator==(const SmsValue&) const;
    bool operator!=(const SmsValue&) const;
    bool operator< (const SmsValue&) const;
    bool operator<=(const SmsValue&) const;
};
```

### 1.3  UI bridge in generated code

`ui.getObject("id")` remains a **runtime call** in V1 (compile-time ID binding
is a V2 optimisation). The glue header wraps the same C callbacks the bridge
already uses:

```cpp
// sms_native_glue.h
SmsValue sms_glue_ui_get_object(const char* id);
SmsValue sms_glue_ui_get(const SmsValue& ref, const char* prop);
void     sms_glue_ui_set(const SmsValue& ref, const char* prop, const SmsValue& val);
SmsValue sms_glue_ui_invoke(const SmsValue& ref, const char* method,
                             std::initializer_list<SmsValue> args);
void     sms_glue_log(const SmsValue& msg);
```

These functions call the registered `g_ui_get_prop` / `g_ui_set_prop` /
`g_ui_invoke` function pointers (same pointers the interpreter uses).
The bridge setup (`SmsBridge::load()`) still runs at startup; only the
interpreter session creation is skipped.

### 1.4  Event handler registration

Each `on target.event(params) { ... }` compiles to a static C++ function
plus a registration entry:

```cpp
// Generated: app_handlers.cpp
#include "sms_native_glue.h"

static SmsValue handler_btn_clicked(SmsCallContext&) {
    // ... body ...
    return SmsValue::Null();
}

void sms_app_register_handlers(SmsHandlerRegistry& reg) {
    reg.add("btn",      "clicked",     handler_btn_clicked);
    reg.add("timeline", "keyframeAdded", handler_timeline_keyframeAdded);
    // ...
}
```

`SmsHandlerRegistry` and `SmsCallContext` are defined in `sms_native_glue.h`.
`SmsBridge` calls `sms_app_register_handlers()` instead of creating a session.

### 1.5  Recursion depth guard

Recursive `fun` calls are guarded at the C++ level:

```cpp
// Generated recursive function
static SmsValue fun_recurse(SmsCallContext& ctx) {
    SmsGlueDepthGuard _guard(ctx);  // throws if depth >= kSmsMaxCallDepth
    return fun_recurse(ctx);
}
```

`SmsGlueDepthGuard` is defined in `sms_native_glue.h`, mirrors the interpreter's
recursion limit (same constant: 256).

### 1.6  `data class` → C++ struct

```sms
data class Point(x = 0, y = 0)
```
generates:
```cpp
struct SmsDataPoint {
    SmsValue x = SmsValue::Int(0);
    SmsValue y = SmsValue::Int(0);
};
SmsValue sms_make_Point(SmsValue x = SmsValue::Int(0),
                         SmsValue y = SmsValue::Int(0));
```

---

## Phase 2: Generated C++ Specification (Golden Examples)

These are the **normative input→output pairs** Codex must satisfy.
They are also the test cases (see Phase 3).

### 2.1  Variable declaration + assignment
```sms
var x = 42
x = x + 1
```
```cpp
SmsValue x = SmsValue::Int(42);
x = x + SmsValue::Int(1);
```

### 2.2  `if` / `else`
```sms
if (x > 0) {
    log.print("positive")
} else {
    log.print("zero or negative")
}
```
```cpp
if ((x > SmsValue::Int(0)).truthy()) {
    sms_glue_log(SmsValue::String("positive"));
} else {
    sms_glue_log(SmsValue::String("zero or negative"));
}
```

### 2.3  `fun` declaration
```sms
fun add(a, b) {
    return a + b
}
```
```cpp
static SmsValue fun_add(SmsCallContext& ctx, SmsValue a, SmsValue b) {
    return a + b;
}
```

### 2.4  Event handler (`on`)
```sms
on btnSave.clicked() {
    var lbl = ui.getObject("feedback")
    lbl.text = "Saved"
}
```
```cpp
static SmsValue handler_btnSave_clicked(SmsCallContext& ctx) {
    SmsValue lbl = sms_glue_ui_get_object("feedback");
    sms_glue_ui_set(lbl, "text", SmsValue::String("Saved"));
    return SmsValue::Null();
}
// registered in sms_app_register_handlers()
```

### 2.5  `ui.getObject` + property read + method call
```sms
on form.load() {
    var vol = ui.getObject("vol")
    var display = ui.getObject("display")
    display.text = vol.value
    vol.focus()
}
```
```cpp
static SmsValue handler_form_load(SmsCallContext& ctx) {
    SmsValue vol     = sms_glue_ui_get_object("vol");
    SmsValue display = sms_glue_ui_get_object("display");
    sms_glue_ui_set(display, "text", sms_glue_ui_get(vol, "value"));
    sms_glue_ui_invoke(vol, "focus", {});
    return SmsValue::Null();
}
```

### 2.6  Null guard
```sms
var x = ui.getObject("maybe")
if (x != null) {
    x.text = "found"
}
```
```cpp
SmsValue x = sms_glue_ui_get_object("maybe");
if ((x != SmsValue::Null()).truthy()) {
    sms_glue_ui_set(x, "text", SmsValue::String("found"));
}
```

### 2.7  String concatenation
```sms
var msg = "Frame: " + frame
```
```cpp
SmsValue msg = SmsValue::String("Frame: ") + frame;
```
*(The `+` operator on `SmsValue` handles mixed int/string by calling `std::to_string` on the numeric side.)*

### 2.8  Unsupported construct → build error
```sms
var f = fun() { 42 }   // first-class function - not supported
```
```
SMS codegen error: first-class function literals are not supported
in the transpiler subset (line 1). Use a named 'fun' declaration instead.
```

---

## Phase 3: Test Specification

Tests live in `SMSCore.Native/tests/sms_codegen_cpp_tests.cpp` and use the
same custom test harness as the existing spec tests.

Each test calls `sms_native_codegen_cpp(source, ...)` and checks that:
- Return code == 0 (success) or != 0 (expected error)
- The generated `.cpp` string contains the expected snippet (substring match)
- OR the error string contains the expected diagnostic

| Test name | Input | Assertion |
|---|---|---|
| `codegen_var_declaration` | `var x = 42` | output contains `SmsValue::Int(42)` |
| `codegen_if_else` | `if (x > 0) { ... }` | output contains `.truthy()` |
| `codegen_fun_declaration` | `fun add(a,b) { return a+b }` | output contains `fun_add` |
| `codegen_event_handler` | `on btn.clicked() { ... }` | output contains `handler_btn_clicked` |
| `codegen_ui_get_object` | `var x = ui.getObject("lbl")` | output contains `sms_glue_ui_get_object` |
| `codegen_property_read` | `var v = x.text` | output contains `sms_glue_ui_get` |
| `codegen_property_write` | `x.text = "hi"` | output contains `sms_glue_ui_set` |
| `codegen_null_guard` | `if (x != null) { ... }` | output contains `SmsValue::Null()` |
| `codegen_string_concat` | `"x=" + x` | output contains `operator+` |
| `codegen_recursion_guard` | recursive `fun` | output contains `SmsGlueDepthGuard` |
| `codegen_data_class` | `data class Point(x=0,y=0)` | output contains `SmsDataPoint` |
| `codegen_unsupported_emits_error` | first-class fun literal | rc != 0, error contains "not supported" |
| `codegen_register_handlers_emitted` | multiple `on` handlers | output contains `sms_app_register_handlers` |
| `codegen_deterministic` | any source | two calls produce identical output |

---

## Phase 4: Implementation (for Codex)

With Phases 1–3 complete, the implementation work for Codex is:

1. **`SMSCore.Native/src/sms_codegen_cpp.cpp`** - AST visitor that emits C++
   according to the golden examples in Phase 2. Each AST node type has one
   `emit_*` method. Unsupported nodes call `emit_error()`.

2. **`SMSCore.Native/include/sms_native_glue.h`** - hand-written runtime glue
   per the spec in section 1.2–1.5. No generated code; ships with the library.

3. **`sms_native.h`** - add `sms_native_codegen_cpp` to the public API.

4. **`run.sh`** - add `--sms-transpile=true` flag; when set, invoke codegen
   before the cmake build step and place generated files in `build/sms_generated/`.

5. **Tests pass**: `ctest -R codegen` green before PR.

---

## Deliverables
- `SMSCore.Native/src/sms_codegen_cpp.cpp` + integration in `sms_native.cpp`
- `SMSCore.Native/include/sms_native_glue.h` - runtime glue header
- `SMSCore.Native/tests/sms_codegen_cpp_tests.cpp` - golden-example tests
- `sms_native.h` - `sms_native_codegen_cpp` export added
- `run.sh` flag `--sms-transpile=true`
- Documentation: supported subset, limits, how to add a new construct

## Acceptance Criteria
- **Profiling gate**: task is only implemented if profiling shows interpreter
  overhead > 5 % of frame time in a representative local app.
- Local app runs with transpiled SMS and no interpreter in the hot path.
- Generated C++ is deterministic for identical SMS input.
- Event handlers and supported control-flow produce behaviour-identical output
  to the interpreter for the defined subset.
- Unsupported constructs abort the build with a clear, actionable error message.
- Interpreted path is unaffected; switching `--sms-transpile` off reverts to
  interpreter with no other changes.
- All tests in `sms_codegen_cpp_tests.cpp` pass.
