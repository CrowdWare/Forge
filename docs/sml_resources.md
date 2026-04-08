# SML Resource System

Resource references let SML properties pull values from named namespaces - most commonly translated strings from locale-aware files.

## Syntax

### Basic reference

```
@Namespace.key
```

Resolves the value of 'key' from the given namespace at runtime. For well-known namespaces (Strings, Colors, Icons, Layouts), no fallback is required - they are resolved from external files. A validation warning is emitted for unknown namespaces without a fallback.

**Example:**
```sml
title: @Strings.windowTitle
```

### Reference with fallback

```
@Namespace.key, "Fallback"
```

Resolves the value if available; uses the literal fallback when the key cannot be resolved. Suppresses validation warnings for unknown namespaces or keys. The fallback must be a string, boolean, integer, or float literal.

**Example:**
```sml
text: @Strings.greeting, "Hello"
```

## Namespaces

Four namespaces are built into SML. Well-known namespaces do not generate validation warnings when no inline block is present, because they can be resolved from external files at runtime.

### Colors

Named color values for theming and styling. Design tokens are defined centrally in theme.sml files and are automatically available in all SML documents without an inline block.

**Resolution order:**

- 1. Inline Colors block in the same SML document (highest priority).
- 2. App-level theme.sml in the same directory as app.sml (optional override).
- 3. ForgeRunner default theme (res://theme.sml).
- 4. Fallback literal (if provided).
- 5. Empty string (with a runtime warning).

**Files:**

| File | Role |
|---|---|
| `theme.sml` | App-level override. Optional. Place next to app.sml to customize tokens. |
| `res://theme.sml` | ForgeRunner built-in default theme. Always loaded as fallback. |

**Example block:**
```sml
Colors {
    primary: "#3A7BD5"
    background: "#1E1E2E"
}
```

### Fonts

Named font paths for use with fontFace and fontWeight properties. Keys follow the convention 'FamilyName-WeightAlias' (e.g. Roboto-Bold). The runtime looks up the key based on the fontFace + fontWeight combination and loads the font via AddThemeFontOverride.

**Resolution order:**

- 1. Inline Fonts block in the same SML document.
- 2. Fallback: warning is logged and no font override is applied.

**Example block:**
```sml
Fonts {
    Roboto-Regular: "res://fonts/Roboto-Regular.ttf"
    Roboto-Bold:    "res://fonts/Roboto-Bold.ttf"
    Roboto-Light:   "res://fonts/Roboto-Light.ttf"
}

Label {
    fontFace: "Roboto"
    fontWeight: bold
    text: "Hello"
}
```

### Icons

Icon resource paths for images and buttons. Resolved from an inline Icons block in the SML document.

**Resolution order:**

- 1. Inline Icons block in the same SML document.
- 2. Fallback literal (if provided).
- 3. Empty string (with a runtime warning).

**Example block:**
```sml
Icons {
    save: "res://assets/icons/save.svg"
    close: "res://assets/icons/close.svg"
}
```

### Layouts

Layout configuration values such as sizes, paddings, and spacing constants. Design tokens are defined centrally in theme.sml files and are automatically available in all SML documents.

**Resolution order:**

- 1. Inline Layouts block in the same SML document (highest priority).
- 2. App-level theme.sml in the same directory as app.sml (optional override).
- 3. ForgeRunner default theme (res://theme.sml).
- 4. Fallback literal (if provided).
- 5. Empty string (with a runtime warning).

**Files:**

| File | Role |
|---|---|
| `theme.sml` | App-level override. Optional. Place next to app.sml to customize tokens. |
| `res://theme.sml` | ForgeRunner built-in default theme. Always loaded as fallback. |

**Example block:**
```sml
Layouts {
    buttonPaddingH: 12
    cornerRadius: 6
}
```

### Strings

Text strings for UI labels, titles, menu entries, and any other displayed text. The Strings namespace is the only one with built-in locale-aware file selection.

**Resolution order:**

- 1. Language-specific file (e.g. strings-de.sml) - overrides for the current system locale.
- 2. Default file (strings.sml) - English or app-default strings.
- 3. Fallback literal in the SML source (if provided).
- 4. Empty string (with a runtime warning).

**Files:**

| File | Role |
|---|---|
| `strings.sml` | Default/English strings. Always loaded. |
| `strings-{lang}.sml` | Locale override, e.g. strings-de.sml for German. Loaded when the system locale matches. |

**Example block:**
```sml
Strings {
    windowTitle: "ForgeRunner"
    menuAppAbout: "About ForgeRunner"
}
```

## Localization (Strings namespace)

The Strings namespace resolves strings from locale-specific files placed alongside the SML document. Files are auto-discovered based on the system locale at startup.

**File format:** A plain SML document containing a single Strings { } block with key-value pairs. Keys are camelCase identifiers; values are quoted strings.

**File placement:** Strings files must be in the same directory as the app.sml that references them.

**Locale selection:** The runtime reads the two-letter ISO 639-1 language code from the system locale (e.g. 'de' from 'de_DE'). It then attempts to load strings-{lang}.sml. If the file does not exist or the locale is 'en', only strings.sml is used.

**Example (strings-de.sml):**
```sml
Strings {
    windowTitle: "ForgeRunner"
    menuAppAbout: "Über ForgeRunner"
    menuAppQuit: "ForgeRunner beenden"
}
```

## Validation

The SML parser emits warnings at parse time for unresolved references:

| Case | Warning emitted? |
|---|---|
| `@Strings.key` - no inline block, no fallback | No (Strings is a well-known external namespace) |
| `@Strings.key` - inline block present, key missing, no fallback | Yes |
| `@UnknownNS.key` - unknown namespace, no fallback | Yes |
| Any reference with a fallback literal | No |

