# SML – Simple Markup Language: DSL Reference

> This documentation describes the complete syntax and semantics of **SML** (Simple Markup Language) as interpreted by the `SmlParser` in SMLCore. It is designed to be loaded as context by AI systems in order to generate valid SML code.

---

## 1. Basic Structure

SML describes a **tree structure of named nodes**. Each node has:
- a **name** (identifier)
- optional **properties** (key-value pairs)
- optional **child nodes** (nested)

### Basic Form

```sml
NodeName {
    propertyName: value
    ChildNode {
        propertyName: value
    }
}
```

### Rules
- A document may have **multiple root nodes**.
- Node names and property names are **case-insensitive** (during parsing).
- Whitespace and line breaks are ignored (except inside strings).
- **Comments** are allowed: `// line comment` and `/* block comment */`.

---

## 2. Value Types

The parser automatically recognizes the following value types based on syntax:

| Type | Example | Description |
|---|---|---|
| `String` | `"Hello World"` | Enclosed in double quotes |
| `Bool` | `true` / `false` | Case-insensitive |
| `Int` | `42`, `-7` | Integer, also negative |
| `Float` | `3.14`, `-0.5` | Decimal number (no exponent, no suffix) |
| `Vec2i` | `1920, 1080` | Two integers, comma-separated |
| `Vec3i` | `255, 128, 0` | Three integers, comma-separated |
| `Padding` | `10, 20, 10, 20` | 1, 2, or 4 integers (top, right, bottom, left) |
| `Identifier` | `myLabel` | Unquoted identifier (schema-dependent) |
| `Enum` | `center` | Unquoted identifier matching an enum value |

### Strings
- Escape sequences in strings: `\n`, `\r`, `\t`, `\"`, `\\`
- Unterminated strings cause a parse error.

### Numbers
- **No** exponent notation (`1e5` is **invalid**)
- **No** numeric suffixes (`42f` is **invalid**)
- Float tuples are **not allowed** (`1.5, 2.5` is **invalid**)

### Vec2i / Vec3i
```sml
pos: 100, 200
size: 1920, 1080
color: 255, 128, 0
```

### Padding (Special Case)
The `padding` property (case-insensitive) is handled specially:
```sml
padding: 10           // all 4 sides = 10
padding: 10, 20       // top/bottom = 10, left/right = 20
padding: 10, 20, 10, 20  // top, right, bottom, left
// 3 values are INVALID
```

---

## 3. Property Types (Schema-Driven)

Some properties have special semantics registered via the schema:

### `id` (Identifier Property)
- Must be a **unique, unquoted identifier** within the entire document.
- Allowed: letters, digits, underscores. Must start with a letter or `_`.
- Alternatively: a single integer as a numeric ID.
- **Duplicates** within the same document cause a parse error.

```sml
Button {
    id: saveButton
    text: "Save"
}
```

### Identifier Properties
- Reference other nodes (e.g. `source`, `target`).
- Written as unquoted identifiers.

### Enum Properties
- Accept predefined, unquoted keywords (e.g. `center`, `left`, `right`).
- Unknown enum values generate a **warning**, not an error.

---

## 4. Anchors (Special Case)

The `anchors` property accepts **one or more identifiers**, separated by `|` or `,`:

```sml
anchors: top | left
anchors: center
anchors: top, right, bottom, left
```

---

## 5. Comments

```sml
// This is a line comment

/* This is a
   block comment */

Window {
    title: "App"  // inline comment
}
```

---

## 6. Complete Syntax Example

```sml
Window {
    title: "My First App"
    pos: 0, 0
    size: 1920, 1080

    // A simple button
    Button {
        id: okBtn
        text: "OK"
        pos: 100, 200
        size: 120, 40
        padding: 8, 16
        visible: true
    }

    Label {
        id: statusLabel
        text: "Ready"
        anchors: top | left
    }

    /* Nested layout */
    VBox {
        spacing: 10
        padding: 20

        Label { text: "Name:" }
        LineEdit {
            id: nameInput
            placeholder: "Please enter..."
        }
        Button {
            id: submitBtn
            text: "Submit"
        }
    }
}
```

---

## 7. Markdown Embedding (MarkdownParser)

SML also supports Markdown blocks as values (via `MarkdownParser`). Markdown is parsed into the following block types:

| Block Type | Markdown Syntax | Properties |
|---|---|---|
| `Heading` | `# H1`, `## H2`, `### H3` | `Text`, `HeadingLevel` (1-3) |
| `Paragraph` | Regular text | `Text` |
| `ListItem` | `- List entry` | `Text` |
| `Image` | `![alt](src)` | `AltText`, `Source` |
| `CodeFence` | `\`\`\`language ... \`\`\`` | `Language`, `Text` |

### Markdown Property Blocks
After each Markdown block, an optional property block may follow:

```markdown
# Heading
{
    color: red
    size: 24
}

- List item
{
    bold: true
}
```

---

## 8. URI Schemas (SmlUriResolver)

Properties containing URIs support the following schemas:

| Schema | Example | Description |
|---|---|---|
| Relative | `images/logo.png` | Relative to the current file |
| `res://` | `res://icons/save.png` | Resource within the project |
| `user://` | `user://config/settings.json` | User data directory |
| `file://` | `file:///home/user/file.txt` | Absolute file path |
| `http://` | `http://example.com/img.png` | HTTP URL |
| `https://` | `https://example.com/img.png` | HTTPS URL |
| `ipfs:/` | `ipfs:/QmHash123/file.png` | IPFS content (mapped to gateway) |

Normalization rules:
- `res:/path` → `res://path` (automatically corrected)
- `user:/path` → `user://path`
- `ipfs://` → `ipfs:/` (canonical form)

---

## 9. Event Scripting (SMS - Simple Multiplatform Script)

SML nodes can be linked to **SMS** scripts. Event binding takes place outside the SML document:

```kotlin
on nodeId.event() {
    // logic here
}
```

Example:
```sml
// SML side
Button {
    id: saveAs
    text: "Save As..."
}
```

```kotlin
// SMS side
on saveAs.clicked() {
    log.info("Save As clicked")
}
```

Typical events: `clicked`, `changed`, `ready`, `pressed`, `released`

---

## 10. Lexical Rules (Summary)

| Token | Rule |
|---|---|
| **Identifier** | `[A-Za-z_][A-Za-z0-9_\-\.]*` (letter or `_`, then alphanumeric, `-`, `_`, `.`) |
| **String** | `"..."` with escape sequences |
| **Int** | Optional `-`, followed by digits |
| **Float** | Optional `-`, digits, `.`, digits (no `e`/`E`) |
| **Bool** | `true` or `false` (case-insensitive) |
| **Comment** | `//` to end of line, or `/* ... */` |
| **Delimiter** | `{` `}` `:` `,` `|` |

**Not allowed:**
- Exponent notation: `1e5`, `2.3E-4`
- Numeric suffixes: `42f`, `10L`
- Float tuples: `1.5, 2.0`
- 3-value padding: `10, 20, 30`
- More than 3 values in a tuple (except `padding` with 4)
- Duplicate IDs within the same document

---

## 11. Separation of Concerns

SML enforces a strict separation:

| Area | Technology | Purpose |
|---|---|---|
| **Structure & UI** | SML | Declaration of layout and content |
| **Behavior & Logic** | SMS (Kotlin-like) | Event handlers and business logic |
| **Native Extensions** | C# / WASM | Plugins, custom runners |
| **Execution** | Forge-Runner | Interpretation and rendering |

---

## 12. Quick Reference for AI Code Generation

When generating SML code, observe the following rules:

1. Introduce **every node** with `NodeName { ... }`.
2. Always write **properties** as `key: value`, never `key = value`.
3. Always enclose **strings** in double quotes: `"text"`.
4. **Booleans** as `true` or `false` (not `"true"`).
5. **Numbers without quotes**: `size: 1920, 1080`.
6. **IDs** always as unquoted identifiers: `id: myButton`.
7. **IDs must be unique across the entire document.**
8. `padding` accepts 1, 2, or 4 values - never 3.
9. Tuples with more than 3 values are forbidden (except `padding` with 4).
10. `anchors` can separate multiple values with `|` or `,`.
11. Comments with `//` or `/* */`.
12. **No semicolons** at the end of properties or nodes.
13. Unquoted identifiers (Enum/Identifier) are only allowed for schema-registered properties.

---

*Documentation generated from SmlParser.cs, SmlSyntax.cs, SmlParserSchema.cs, SmlUriResolver.cs and MarkdownParser.cs – SMLCore © 2026 CrowdWare, GPLv3*
