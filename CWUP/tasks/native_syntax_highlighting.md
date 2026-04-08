# Native Syntax Highlighting for CodeEdit

## Goal
Port `CodeEditSyntaxRuntime.cs` to C++ so that `CodeEdit` controls in
`ForgeRunner.Native` get syntax highlighting for SML, SMS, Markdown, and C#.

## Context
C# sets up `CodeHighlighter` resources on `CodeEdit` nodes based on a `language:`
property. The keyword lists and color rules are defined in `ForgeRunner/syntax/*.cs`.

In `forge_ui_builder.cpp`, `CodeEdit` is created but no syntax highlighter is attached.

## Implementation

### language property → highlighter
In `apply_props()` for `CodeEdit`, read `language:` property and call
`attach_syntax_highlighter(code_edit, lang)`.

### SML highlighter
Keywords (node types from schema), operators (`{`, `}`, `:`), string literals,
comments (`//`), resource refs (`@`).

### SMS highlighter
Keywords: `fun`, `var`, `get`, `set`, `when`, `if`, `else`, `while`, `for`, `in`,
`break`, `continue`, `return`, `true`, `false`, `null`.
Operators, string literals, comments (`//`).

### Markdown highlighter
Headings (`#`), bold/italic (`**`, `_`), code spans (`` ` ``), links (`[]()`).

### C# highlighter
Standard C# keywords, string literals, comments (`//`, `/* */`), preprocessor (`#`).

### Godot API
Use `godot::CodeHighlighter`:
```cpp
auto* hl = memnew(CodeHighlighter);
hl->add_keyword_color("fun", Color(0.4f, 0.7f, 1.0f));
// ...
code_edit->set_syntax_highlighter(hl);
```

## Acceptance Criteria
- `CodeEdit { language: sml }` highlights SML keywords in the accent color.
- `CodeEdit { language: sms }` highlights SMS keywords.
- `CodeEdit { language: markdown }` highlights headings and emphasis.
- No syntax highlighter on unknown `language:` value - no crash.

## Reference
- C#: `ForgeRunner/Runtime/UI/CodeEditSyntaxRuntime.cs`
- C#: `ForgeRunner/syntax/sml_syntax.cs`
- C#: `ForgeRunner/syntax/sms_syntax.cs`
- C#: `ForgeRunner/syntax/markdown_syntax.cs`
- C#: `ForgeRunner/syntax/cs_syntax.cs`
