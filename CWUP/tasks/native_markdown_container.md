# Native Markdown Container

## Goal
Port `MarkdownContainer.cs` and `MarkdownParser.cs` to C++ so that SML `Markdown`
nodes render formatted text in `ForgeRunner.Native`.

## Context
The C# implementation parses a markdown string into a sequence of block elements
(headings, paragraphs, code blocks, lists, horizontal rules) and produces a
`VBoxContainer` of `RichTextLabel` / `PanelContainer` children.

In `forge_ui_builder.cpp` the `Markdown` node type is either absent or falls back
to a plain `Label`.

## Subsystems to Port

### MarkdownParser (C++ standalone)
Input: `std::string` markdown source.
Output: `std::vector<MarkdownBlock>` where `MarkdownBlock` is:
```cpp
enum class BlockKind { Heading, Paragraph, CodeBlock, ListItem, HRule, Image };
struct MarkdownBlock {
    BlockKind kind;
    int level = 0;          // heading level 1–6
    std::string text;       // raw text (may contain inline markup)
    std::string lang;       // code block language hint
};
```
Inline formatting (`**bold**`, `_italic_`, `` `code` ``, `[link](url)`) is handled
by `RichTextLabel`'s BBCode output.

### Markdown → BBCode conversion
Convert inline markdown to Godot BBCode for `RichTextLabel::set_use_bbcode(true)`:
- `**text**` → `[b]text[/b]`
- `_text_` → `[i]text[/i]`
- `` `code` `` → `[code]text[/code]`
- `[label](url)` → `[url=url]label[/url]`

### ForgeBgVBoxContainer as root
Each `Markdown` SML node builds a `VBoxContainer` (or `ForgeBgVBoxContainer` when
bg properties are present). Each block becomes a child:
- Heading → `Label` with `theme_font_size` scaled by level.
- Paragraph → `RichTextLabel` with BBCode.
- Code block → `PanelContainer` + `CodeEdit` (read-only, syntax highlighting off).
- List item → `HBoxContainer` + bullet `Label` + `RichTextLabel`.
- HRule → `HSeparator`.

## Properties to Support
- `text:` - inline markdown string or `@Strings.key` ref.
- `src:` - path to external `.md` file to load and render.
- `fontSize:` - base font size (headings scale from this).
- `bgColor:`, `borderRadius:`, etc. - forwarded to root VBoxContainer.

## Acceptance Criteria
- A `Markdown { text: "# Hello\nWorld" }` node renders a large heading and a paragraph.
- Fenced code blocks render in a styled box.
- `src: "readme.md"` loads and renders an external file.

## Reference
- C#: `ForgeRunner/Runtime/UI/MarkdownContainer.cs`
- C#: `ForgeRunner/Runtime/Sml/MarkdownParser.cs`
