# SMS – Simple Multiplatform Script
## DSL Reference for AI Code Generation

> This document is the authoritative reference for generating valid SMS scripts.
> Feed this entire document to an AI model to enable reliable SMS code generation.

---

## Overview

SMS (Simple Multiplatform Script) is the scripting layer of the **Forge** platform.
It connects SML UI declarations to native logic using a minimal, Kotlin-inspired syntax.
SMS is **event-first**: instead of wiring dispatcher chains, you bind handlers directly to UI element events using `on <id>.<event>(...) { }`.

**SMS engine version:** 1.1.0

---

## File Structure

An SMS file is a flat sequence of:
1. `var` declarations (top-level state)
2. `fun` function declarations
3. `data class` declarations
4. `on` event handler declarations
5. Imperative statements (executed top-level at load time)

Order matters: `fun`, `data class`, and `on` declarations are hoisted and registered before any imperative statements run.
Duplicate event handlers for the same `id.event` pair are a **parse error**.

---

## Lexical Rules

- **Comments:** `// line comment` or `/* block comment */`
- **Whitespace:** spaces, tabs, and `\r` are ignored; `\n` (newline) is a statement terminator
- **Semicolons** may also terminate statements (optional)
- **Identifiers:** `[a-zA-Z_][a-zA-Z0-9_]*`
- **Keywords (reserved):** `fun var get set when if else while for in break continue return on true false null data class`

---

## Types & Values

SMS is dynamically typed. There are five runtime value types:

| Type | Literal examples | Truthy when |
|------|-----------------|-------------|
| `Number` | `42`, `3.14`, `-7` | `!= 0` |
| `String` | `"hello"`, `"line\n"` | length `> 0` |
| `Boolean` | `true`, `false` | `== true` |
| `Null` | `null` | never |
| `Array` | `[1, "a", true]` | has elements |
| `Object` | (from `data class`) | always |

**Number notes:** All numbers are `double` internally. Integer division (`/`) truncates: `7 / 2 == 3`.
Division by zero throws a `RuntimeError`.

---

## Variables

```kotlin
var name = "Alice"
var count = 0
var items = [1, 2, 3]
var active = true
var nothing = null
```

Variables are mutable. There is no `val`/`const`.

### Computed Properties (get/set)

A variable can have a custom getter and/or setter:

```kotlin
var temperature = 0
get { field * 1.8 + 32 }        // returns Fahrenheit; 'field' = raw stored value
set(v) { field = v - 32 / 1.8 } // 'field' is writable inside setter; parameter name is 'v'
```

- Inside `get`: `field` refers to the stored value (read-only recommended)
- Inside `set(param)`: `field` can be mutated to change the stored value; `param` is the incoming value
- The getter **return value** is used when the variable is read

---

## Operators

### Arithmetic
| Operator | Meaning |
|----------|---------|
| `+` | Add (numbers) or concatenate (strings, or any mix) |
| `-` | Subtract |
| `*` | Multiply |
| `/` | Integer-truncating divide |

### Comparison
`==`, `!=`, `<`, `<=`, `>`, `>=`

### Logical
`&&` (and), `||` (or), `!` (not)

### Increment / Decrement (postfix only)
`x++`, `x--` - returns old value, then mutates variable

### String Concatenation
`+` between a string and any other type coerces the other to string:
```kotlin
var msg = "Count: " + count   // "Count: 42"
```

---

## String Interpolation

```kotlin
var name = "World"
var greeting = "Hello, $name!"           // simple identifier
var expr = "Result: ${1 + 2 * 3}"        // arbitrary expression
var multi = "Hi ${name}, you have $count messages."
```

- `$identifier` - inlines a simple variable
- `${expression}` - inlines any expression
- Escape with `\$` to use a literal dollar sign

**Escape sequences inside strings:**
`\n` `\t` `\r` `\\` `\"` `\$`

---

## Control Flow

### if / else

```kotlin
if (x > 10) {
    log.info("big")
} else {
    log.info("small")
}
```

`if` can also be used as an expression:
```kotlin
var label = if (score > 50) "pass" else "fail"
```

### while

```kotlin
var i = 0
while (i < 10) {
    log.info(i)
    i++
}
```

### for (C-style)

```kotlin
for (var i = 0; i < 5; i++) {
    log.info(i)
}
```

### for-in (iterate array)

```kotlin
var colors = ["red", "green", "blue"]
for (color in colors) {
    log.info(color)
}
```

### break / continue

Work inside `while` and `for` loops as expected.

### when (pattern match)

`when` with a subject (equality matching):
```kotlin
var lang = os.getLocale()
when (lang) {
    "de" -> log.info("Hallo!")
    "fr" -> log.info("Bonjour!")
    else -> log.info("Hello!")
}
```

`when` without a subject (boolean conditions):
```kotlin
when {
    x < 0  -> log.info("negative")
    x == 0 -> log.info("zero")
    else   -> log.info("positive")
}
```

`when` as expression:
```kotlin
var msg = when (lang) {
    "de" -> "Hallo"
    "es" -> "Hola"
    else -> "Hello"
}
```

---

## Functions

```kotlin
fun greet(name) {
    return "Hello, " + name + "!"
}

fun add(a, b) {
    return a + b
}

// Call
var result = greet("Alice")
var sum = add(3, 4)
```

- Parameters are positional, no default values
- `return` exits early; last evaluated expression is **not** implicitly returned (use explicit `return`)
- Functions can call each other (hoisted)
- Recursion is supported

---

## Data Classes

Lightweight value objects (similar to Kotlin data classes):

```kotlin
data class Point(x, y)
data class User(name, age, email)

// Instantiate (positional arguments)
var p = Point(10, 20)
var u = User("Alice", 30, "alice@example.com")

// Field access
log.info(p.x)          // 10
log.info(u.name)       // Alice

// Field mutation
p.x = 99
u.age = 31
```

---

## Arrays

```kotlin
var list = [10, 20, 30]

// Index access (0-based)
var first = list[0]

// Index assignment
list[0] = 99

// Methods
list.add(40)             // append
list.remove(20)          // remove by value (returns bool)
list.removeAt(1)         // remove by index (returns removed value)
list.contains(30)        // returns bool

// Property
var n = list.size        // member access, not a method call
```

---

## String Methods

Called as methods on string values:

```kotlin
var s = "  Hello World  "
s.length           // returns number (no parentheses needed - actually a method call with no args)
s.toUpperCase()    // "  HELLO WORLD  "
s.toLowerCase()    // "  hello world  "
s.trim()           // "Hello World"
```

---

## Built-in Functions

These are globally available without any namespace prefix:

| Function | Signature | Description |
|----------|-----------|-------------|
| `toString` | `toString(value)` | Converts any value to String |
| `size` | `size(array)` | Returns array length as Number |
| `isNumber` | `isNumber(value)` | Returns Boolean |
| `isString` | `isString(value)` | Returns Boolean |
| `isBoolean` | `isBoolean(value)` | Returns Boolean |
| `isNull` | `isNull(value)` | Returns Boolean |
| `isArray` | `isArray(value)` | Returns Boolean |
| `abs` | `abs(number)` | Absolute value |
| `min` | `min(a, b)` | Smaller of two numbers |
| `max` | `max(a, b)` | Larger of two numbers |
| `print` | `print(value)` | Writes to stdout (no newline) |

---

## Native Namespace Objects

The Forge Runner registers additional objects as native variables.
These are accessed via dot notation: `namespace.method(args)`.

The following namespaces are provided by the platform (registered externally via `ScriptEngine.RegisterFunction`):

### `log` - Logging

```kotlin
log.info("Message")
log.warn("Warning")
log.error("Something failed")
log.debug("Debug value: " + x)
```

### `os` - Operating System

```kotlin
var locale = os.getLocale()     // e.g. "de", "en", "fr"
var platform = os.getPlatform() // e.g. "windows", "macos", "linux"
```

### `fs` - File System (ProjectFs)

```kotlin
fs.readText("data/config.txt")          // returns String
fs.writeText("output/log.txt", content) // returns null
fs.exists("assets/image.png")           // returns Boolean
fs.createDir("output/subfolder")        // returns null
fs.delete("temp/file.txt")              // returns null
// fs.list("dir") returns Array of path strings
```

> **Note:** `fs` is sandboxed to the project root. Absolute paths and `..` traversal are rejected.

### `ui` - UI Interaction

```kotlin
ui.setText("labelId", "New Text")
ui.setVisible("panelId", false)
ui.setEnabled("buttonId", true)
ui.getValue("inputId")        // returns current value of a UI element
```

> The exact API of `log`, `os`, `fs`, and `ui` depends on what the host Runner registers. The above represents the standard Forge interface. Always check with the platform host if unsure.

---

## Event Handlers

This is the primary integration mechanism between SML UI and SMS logic.

### Syntax

```kotlin
on <targetId>.<eventName>(<params>) {
    // body
}
```

- `targetId` - the `id` of an SML element
- `eventName` - the event name (e.g. `clicked`, `changed`, `ready`, `textChanged`)
- `params` - zero or more parameter names (matched positionally to event args)

### Common Events

| Event | Parameters | Fired when |
|-------|-----------|------------|
| `ready()` | - | Script/scene finishes loading |
| `clicked()` | - | Button or item is clicked |
| `changed(value)` | new value | Input value changes |
| `textChanged(text)` | new text | Text field content changes |
| `selected(index)` | selected index | List selection changes |

### Examples

```kotlin
// No parameters
on saveButton.clicked() {
    fs.writeText("save.txt", ui.getValue("editor"))
    log.info("Saved!")
}

// With parameter
on nameInput.textChanged(text) {
    ui.setText("greetingLabel", "Hello, " + text + "!")
}

// ready handler (runs at startup)
on app.ready() {
    var lang = os.getLocale()
    var msg = when (lang) {
        "de" -> "Hallo Welt!"
        "fr" -> "Bonjour le monde!"
        else -> "Hello World!"
    }
    log.info(msg)
}
```

### super() - Delegating to Platform Default

Inside an event handler, `super(args...)` forwards the event to the platform's built-in handler:

```kotlin
on closeButton.clicked() {
    log.info("Closing...")
    super()   // let the Runner handle the actual close
}
```

### Rules

- Only **one** handler per `id.event` combination (duplicate = parse error)
- Event handlers are **not** functions and cannot be called manually
- Parameters must match the number of arguments the event provides

---

## Complete Example Scripts

### 1. Startup Locale Greeting

```kotlin
fun getGreeting(lang) {
    return when (lang) {
        "de" -> "Hallo Welt!"
        "es" -> "¡Hola Mundo!"
        "fr" -> "Bonjour le monde!"
        "pt" -> "Olá Mundo!"
        else -> "Hello World!"
    }
}

on app.ready() {
    var lang = os.getLocale()
    var msg = getGreeting(lang)
    log.info(msg)
    ui.setText("welcomeLabel", msg)
}
```

### 2. Counter with UI

```kotlin
var count = 0

on incrementBtn.clicked() {
    count++
    ui.setText("counterLabel", "Count: " + count)
}

on decrementBtn.clicked() {
    count--
    ui.setText("counterLabel", "Count: " + count)
}

on resetBtn.clicked() {
    count = 0
    ui.setText("counterLabel", "Count: 0")
}
```

### 3. Form Validation

```kotlin
fun isValidEmail(email) {
    return email.length > 5 && email.contains("@")
}

on submitBtn.clicked() {
    var name = ui.getValue("nameInput")
    var email = ui.getValue("emailInput")

    if (name.trim().length == 0) {
        ui.setText("errorLabel", "Name is required.")
        return
    }

    if (!isValidEmail(email)) {
        ui.setText("errorLabel", "Invalid email address.")
        return
    }

    ui.setText("errorLabel", "")
    log.info("Form submitted: " + name + " / " + email)
}
```

### 4. Data Class Usage

```kotlin
data class Product(name, price, inStock)

var catalog = [
    Product("Widget", 9.99, true),
    Product("Gadget", 24.99, false),
    Product("Doohickey", 4.49, true)
]

on listView.selected(index) {
    var item = catalog[index]
    ui.setText("nameLabel", item.name)
    ui.setText("priceLabel", "$" + toString(item.price))
    ui.setVisible("outOfStockBanner", !item.inStock)
}
```

### 5. File Read/Write

```kotlin
on loadBtn.clicked() {
    if (fs.exists("notes.txt")) {
        var content = fs.readText("notes.txt")
        ui.setText("editor", content)
        log.info("Loaded notes.txt")
    } else {
        ui.setText("editor", "")
        log.warn("notes.txt not found")
    }
}

on saveBtn.clicked() {
    var content = ui.getValue("editor")
    fs.writeText("notes.txt", content)
    log.info("Saved.")
}
```

---

## Error Types

| Error | When thrown |
|-------|-------------|
| `LexError` | Unexpected character, unterminated string/comment |
| `ParseError` | Syntax error, duplicate event handler |
| `RuntimeError` | Undefined variable/function, wrong argument count, type mismatch, division by zero, out-of-bounds array access, symlink escape |

---

## Gotchas & Rules for Code Generation

1. **No implicit return** - always use explicit `return` in functions to return a value.
2. **Integer division** - `/` truncates: `7 / 2 == 3`. Use multiplication workarounds for float math.
3. **Postfix only** - `++` and `--` are **postfix** only (`x++` ✅, `++x` ❌).
4. **`&&` and `||` only** - single `&` or `|` is a lex error.
5. **No `val`** - all variables declared with `var` are mutable.
6. **`size` on arrays** - use `.size` as a member (no parentheses), or `size(array)` as a global function.
7. **String methods need `()`** - `s.toUpperCase()`, `s.toLowerCase()`, `s.trim()` require parentheses; `s.length` does not.
8. **One handler per event** - duplicating `on foo.clicked()` is a parse error.
9. **`for-in` only iterates arrays** - using it on a non-array throws at runtime.
10. **`super()` only inside event handlers** - calling it elsewhere throws.
11. **No closures** - functions and event handlers do not capture lexical scope; use `var` at the appropriate scope level.
12. **No type annotations** - SMS is fully dynamically typed; do not write type hints.
13. **Newlines matter** - a newline ends a statement. Multi-line expressions must continue on the same line or use `{ }` blocks.
14. **No `null` coalescing** - check `isNull(x)` or `x == null` explicitly.

---

## Grammar Summary (EBNF-style)

```
program        = statement* EOF
statement      = varDecl | funDecl | dataDecl | ifStmt | whileStmt
               | forStmt | forInStmt | breakStmt | continueStmt
               | returnStmt | onDecl | assignment | exprStmt

varDecl        = "var" IDENT "=" expr newline? ( getter | setter )*
getter         = "get" "{" expr "}"
setter         = "set" "(" IDENT ")" "{" stmtList "}"
funDecl        = "fun" IDENT "(" paramList ")" "{" stmtList "}"
dataDecl       = "data" "class" IDENT "(" fieldList ")"
onDecl         = "on" IDENT "." IDENT "(" paramList ")" "{" stmtList "}"
ifStmt         = "if" "(" expr ")" "{" stmtList "}" ( "else" "{" stmtList "}" )?
whileStmt      = "while" "(" expr ")" "{" stmtList "}"
forStmt        = "for" "(" ( varDecl | assignment ) ";" expr ";" ( assignment | exprStmt ) ")" "{" stmtList "}"
forInStmt      = "for" "(" IDENT "in" expr ")" "{" stmtList "}"
breakStmt      = "break"
continueStmt   = "continue"
returnStmt     = "return" expr?
assignment     = target "=" expr
exprStmt       = expr

expr           = whenExpr | ifExpr | logicOr
whenExpr       = "when" ( "(" expr ")" )? "{" whenBranch+ "}"
whenBranch     = ( expr | "else" ) "->" expr newline?
ifExpr         = "if" "(" expr ")" expr "else" expr
logicOr        = logicAnd ( "||" logicAnd )*
logicAnd       = equality ( "&&" equality )*
equality       = comparison ( ( "==" | "!=" ) comparison )*
comparison     = addition ( ( "<" | "<=" | ">" | ">=" ) addition )*
addition       = multiply ( ( "+" | "-" ) multiply )*
multiply       = unary ( ( "*" | "/" ) unary )*
unary          = ( "!" | "-" | "+" ) unary | postfix
postfix        = primary ( "++" | "--" )?
primary        = NUMBER | STRING | INTERPOLATED_STRING | BOOL | "null"
               | IDENT | "(" expr ")"
               | primary "[" expr "]"       // array access
               | primary "." IDENT          // member access
               | primary "." IDENT "(" args ")"  // method call
               | IDENT "(" args ")"         // function call
               | "[" ( expr ( "," expr )* )? "]" // array literal
               | "super" "(" args ")"

target         = IDENT | primary "." IDENT | primary "[" expr "]"
paramList      = ( IDENT ( "," IDENT )* )?
fieldList      = IDENT ( "," IDENT )*
args           = ( expr ( "," expr )* )?
stmtList       = statement*
newline        = "\n" | ";"
```

---

*Generated from SMSCore source: Lexer.cs, Parser.cs, Interpreter.cs, Ast.cs, Value.cs, NativeFunction.cs - CrowdWare 2026*
