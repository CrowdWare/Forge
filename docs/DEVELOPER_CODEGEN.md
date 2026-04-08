# Developer Guide: Code Generation Pipeline

This document explains how code/documentation generation works in this repository and how to extend it with new SML elements and SMS runtime functions.

## Overview

The generation pipeline is split into two Godot scripts:

- `tools/generate_sml_element_docs.gd`
- `tools/generate_sms_functions_docs.gd`

They are run via:

```bash
/Users/art/SourceCode/Forge/run_runner.sh docs
```

## Outputs

### From `generate_sml_element_docs.gd`

- `docs/SML/Elements/*.md`
  - Per-element developer docs (inheritance, properties, events, actions, pseudo children).
- `ForgeRunner/Generated/SchemaTypes.cs`
- `ForgeRunner/Generated/SchemaProperties.cs`
- `ForgeRunner/Generated/SchemaEvents.cs`
- `ForgeRunner/Generated/SchemaContextProperties.cs`
- `ForgeRunner/Generated/SchemaLayoutAliases.cs`
  - Runtime mapping tables used by Forge-Runner.

### From `generate_sms_functions_docs.gd`

- `docs/sms_functions.md`
- `ForgeRunner/Generated/SchemaFunctions.cs`

## Data Sources

### 1) Godot ClassDB (runtime UI controls)

`generate_sml_element_docs.gd` introspects ClassDB for supported UI classes and extracts:

- inheritance chains
- supported editable properties
- signals/events
- callable methods (for docs)

### 2) Manual specs (`tools/specs/*.gd`)

Specs are used for manual/custom runtime elements and extra metadata.

Current examples:

- `tools/specs/markdown.gd`
- `tools/specs/viewport3d.gd`
- `tools/specs/functions.gd` (SMS built-in functions)
- `tools/specs/context_properties.gd` (attached/context properties by parent-child relation)
- `tools/specs/layout_aliases.gd` (canonical layout properties + aliases)

## Naming Rules

- Godot names are usually `snake_case`.
- SMS/SML exposed names are normalized to `lowerCamelCase`.
- The conversion is centralized in helper logic (`_to_lower_camel(...)`).

## Runtime Contract

Generated files under `ForgeRunner/Generated` are the runtime source of truth.

- `SchemaTypes`: known type set and parent relation.
- `SchemaProperties`: SML property name -> Godot property name + type.
- `SchemaEvents`: SMS event name -> Godot signal + parameter metadata.
- `SchemaContextProperties`: SML attached/context properties (`parent + child + property`).
- `SchemaLayoutAliases`: canonical layout alias mapping and apply mode (`whole`/`x`/`y`).
- `SchemaFunctions`: built-in SMS helper functions.

Do not add hardcoded per-control mapping tables in runtime code if it can be represented in generated schema data.

## Add a New SML Element (Spec-based)

Create a new file under `tools/specs`, for example `tools/specs/my_widget.gd`:

```gdscript
extends RefCounted

func get_spec() -> Dictionary:
    return {
        "name": "MyWidget",
        "backing": "Control",
        "properties": [
            {"sml":"id", "type":"identifier", "default":"-"},
            {"sml":"title", "type":"string", "default":"\"\""},
        ],
        "actions": [
            {"sms":"refresh", "params":[], "returns":"void"}
        ],
        "notes": [
            "Optional notes for generated element docs."
        ],
        "examples_sml": [
            "MyWidget {",
            "    id: demo",
            "    title: \"Hello\"",
            "}"
        ]
    }
```

Then regenerate docs/schema:

```bash
/Users/art/SourceCode/Forge/run_runner.sh docs
```

## Add a New SMS Runtime Function

Edit `tools/specs/functions.gd` and add an entry to `FUNCTIONS`.

Required fields:

- `category`
- `signature`
- `description`

Regenerate:

```bash
/Users/art/SourceCode/Forge/run_runner.sh docs
```

This updates:

- `docs/sms_functions.md`
- `ForgeRunner/Generated/SchemaFunctions.cs`

## Validation Checklist

After any generator/spec change:

1. Regenerate artifacts (`run_runner.sh docs`).
2. Build solution:

   ```bash
   dotnet build /Users/art/SourceCode/Forge/Forge.sln
   ```

3. Verify only expected files changed (docs and generated schema).
4. Commit generator/spec changes together with regenerated outputs.

## Guardrails

- Do not manually edit `ForgeRunner/Generated/*.cs`.
- Keep generator output deterministic (stable sorting).
- Keep runtime behavior data-driven via generated schema files.
- Prefer spec additions over ad-hoc runtime special cases.
