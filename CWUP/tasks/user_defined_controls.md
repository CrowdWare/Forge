# UserDefined Controls

In the following sample we use a similar Control and children multiple times.
It would be nice if we can declare such a Control once and reuse it somewhere else.

We should be able to define it in the same .sml or in a separate .sml and include it for rendering.
QML which is the idea giver for SML is doing that already.

It can also be useful, when an included Control has got its own .sms script in order to be able to handle events. Naming conventions like in all other SMLs:
```
<name>.sml
<name>.sms
```

## Design

### Component Definition – no extra keyword

A component is defined like any other top-level block. The parser distinguishes top-level blocks by name:

| Block | Behandlung |
|---|---|
| `Window`, `Dialog`, `Page` | Root (rendered) |
| `Strings`, `Colors`, `Icons`, `Layouts`, `Fonts`, `Elevations` | Resources |
| anything else | Component definition |

### Property Declaration

Properties are declared inside the component block with the `property` keyword.
No types needed – the type is inferred from the default value.

```sml
NavTab {
    property text:   "Tab"    // String prop with default
    property active: false    // Bool prop with default
    property tabId:  id       // Identifier prop (id = unquoted keyword signals identifier type)

    Control {
        shrinkH: true
        width: 120
        height: 44

        TextureButton {
            id: {tabId}
            textureNormal: "res://assets/images/button_normal.png"
            texturePressed: "res://assets/images/button_focused.png"
            textureHover: "res://assets/images/button_focused.png"
            width: 120
            height: 44
            ignoreTextureSize: true
            toggleMode: {active}
        }
        TextureRect {
            src: "appRes://logo.svg"
            top: 15
            left: 10
            width: 30
            height: 30
            mouseFilter: ignore
        }
        Label {
            top: 15
            left: 45
            text: {text}
            mouseFilter: ignore
        }
    }
}

Window {
    HBoxContainer {
        NavTab { tabId: tabStart    text: "Start"   active: true }
        NavTab { tabId: tabLearn    text: "Learn" }
        NavTab { tabId: tabDiscover text: "Discover" }
        NavTab { tabId: tabUpdates  text: "Updates" }
    }
}
```

### Prop Reference Syntax: `{propName}`

Inside a component body, props are referenced with `{propName}`.
This is unambiguous because:
- After `:` the parser is in value context (not block context)
- `{` in value context → prop reference
- `{` after type name → block start (existing behavior)

### Namespaces → Directory Path

Components can be organized in subdirectories using dot-notation:

| Syntax | File loaded |
|---|---|
| `NavTab {}` | `./navtab.sml` (same directory) |
| `ui.NavTab {}` | `./ui/navtab.sml` |
| `ui.cards.Card {}` | `./ui/cards/card.sml` |

- Namespace separator `.` maps to `/`
- Component name is lowercased for the filename

The script file follows the same convention:
```
ui/navtab.sml   → Component definition
ui/navtab.sms   → Event handler for this component
```

### Lookup Order (inline wins)

1. Inline definition in the same document → always wins
2. Same directory: `navtab.sml` (no namespace)
3. Subdirectory: `ui/navtab.sml` (with namespace)

### Inheritance from Godot Types

A component can declare a Godot base type via `inheritance:`. This eliminates the explicit root element wrapper and moves properties + children directly onto the synthesized base node.

```sml
// Without inheritance - explicit root required
NavTab {
    property text: "Tab"
    Control {
        width: 120
        Label { text: {text} }
    }
}

// With inheritance - no wrapper needed
NavTab {
    property text:   "Tab"
    property tabId:  id
    inheritance:     Control     // Godot type (built-in or C# custom)

    shrinkH: true                // default properties of the created Control
    width: 120
    height: 44

    TextureButton { id: {tabId} }
    Label { text: {text} }
}
```

**Rules:**
- `inheritance:` takes an unquoted Godot type name (e.g. `Control`, `VBoxContainer`, `Button`)
- All non-`property`, non-`inheritance` properties in the component block → become defaults on the synthesized root node
- Children in the component block → become children of the synthesized root node
- PropRefs (`{propName}`) work on body-level properties too: `width: {size}`
- Only Godot built-in types and C# custom Godot nodes - no component-to-component inheritance
- Without `inheritance:`, exactly one explicit child element is required (existing behaviour)

### File-based Component

A file-based component (`ui/navtab.sml`) uses the exact same syntax as an inline definition:

```sml
NavTab {
    property text:   "Tab"
    property tabId:  id
    property active: false

    Control {
        // ... same body as inline
    }
}
```

---

## Implementation

### Files to change

| File | Change |
|---|---|
| `SMLCore/Runtime/Sml/SmlSyntax.cs` | Add `SmlComponentDef`, `SmlValueKind.PropRef`, `SmlDocument.Components` |
| `SMLCore/Runtime/Sml/SmlParser.cs` | Top-level dispatch for component defs, `property` keyword, `{name}` value parsing, dotted type names |
| `ForgeRunner/Runtime/UI/SmlUiBuilder.cs` | Component lookup, prop substitution, instantiation |
| `SMLCore.Tests/SmlParserTests.cs` | Tests for new syntax |

### Parser changes

1. **Top-level dispatch**: if block name is not a known root/resource type → `SmlDocument.Components[name]`
2. **`property` keyword**: inside a component block, `property name: value` → `SmlComponentDef.Props`
3. **`{identifier}` value**: in value context after `:` → `SmlValue.FromPropRef(name)`
4. **Dotted type name**: `ui.NavTab { }` → namespace `"ui"`, type `"NavTab"`

### AST additions (`SmlSyntax.cs`)

```csharp
// New value kind
SmlValueKind.PropRef   // {propName}

// New document-level structure
public class SmlComponentDef
{
    public string Name { get; }
    public string? Namespace { get; }
    public Dictionary<string, SmlValue> Props { get; }  // name → default value
    public SmlNode Body { get; }                        // the root Control/Container
}

// SmlDocument gets:
public Dictionary<string, SmlComponentDef> Components { get; }
```

### UiBuilder changes (`SmlUiBuilder.cs`)

1. Before build: collect all inline component definitions from `document.Components`
2. When encountering unknown element type:
   - Check inline components
   - If not found and namespace present: load `<namespace>/<typename>.sml` from app directory
   - Parse and cache the external component
3. Instantiate: substitute `PropRef` values with caller-provided property values (or defaults)
4. Build the subtree recursively

---

## Original Problem (motivating example)

The nav tabs in `docs/Default/main.sml` repeat the same structure 4× with only `id` and `text` changing:

```sml
Control {
    shrinkH: true
    width: 120
    height: 44

    TextureButton {
        id: tabStart
        textureNormal: "res://assets/images/button_normal.png"
        texturePressed: "res://assets/images/button_focused.png"
        shrinkH: true
        toggleMode: true
    }
    TextureRect {
        src: "appRes://logo.svg"
        top: 15
        left: 10
        width: 30
        height: 30
    }
    Label {
        top: 15
        left: 45
        text: "Start"
    }
}
Control {
    // ... repeated 3 more times
}
```
