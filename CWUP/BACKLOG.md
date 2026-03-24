# Backlog

## Functional Requirements

### Platform Support
- The Forge-Designer must run on Windows, macOS, and Linux.
- Feature parity across platforms (no platform-exclusive features).
- Platform-specific integrations must degrade gracefully on unsupported systems.
- The Forge-Runner must run on Android in addition to desktop platforms.

## Non-Functional Requirements

### Performance
- Stable frame pacing; avoid stutter (consistent frame times).
- Scalable quality settings (low / medium / high) and resolution scaling.
- Predictable CPU/GPU budgets per frame.
- Heavy I/O, parsing, and preparation tasks may run on worker threads; all scene graph mutations must occur on the main thread.

### Determinism & Reproducibility
- Fixed and well-defined tick rates for gameplay simulation.
- Simulation logic decoupled from render frame rate.
- Repeatable combat outcomes driven by server state and validation.
- Clients are non-authoritative and act as presentation and input layers.

### Stability & Fault Tolerance
- No hard crashes on malformed SML, scripts, or content.
- Graceful failure with clear error logs and safe fallbacks.
- Unknown or unsupported SML entries must produce warnings, not fatal errors.
- Clean shutdown on SIGTERM; always flush and close saves safely.

### Security (Self-Hosted / Guild Servers)
- Validate all client inputs on the server.
- Rate limiting for spam-prone actions (abilities, RPCs, join/leave).
- Business logic executes server-side only.
- Clear separation between admin and player privileges.
- No remote code execution through content or configuration.
- Sandbox security rules (e.g., ProjectFS root jail, no absolute paths, no traversal, no symlink escapes) must be strictly enforced and must not be bypassed.
- No backdoors or implicit bypass paths may be introduced; all new features must explicitly respect the existing security model.

### Maintainability
- KISS principle: one scripting and behavior system for bots, bosses, doors, traps, switches, and interactions.
- Minimal special cases; behavior is defined declaratively and bound consistently.
- Versioned network protocol and save formats with explicit migration paths.

### Observability
- Structured logs including timestamp, level, subsystem, and entity identifier.
- Lightweight debug overlays for demo and training modes (e.g., DPS, threat, cooldowns, CC timers).
- Simple server status output (uptime, connected players, tick duration).

### Configurability
- All runtime settings defined via configuration files.
- Optional hot reload for SML and configuration files in local and development modes.
- Explicitly documented non-goals to manage scope and expectations.

### Portability
- Guild server runs on common Linux VPS distributions.
- Reproducible builds with pinned toolchains where possible.

### Data & Persistence
- Atomic save writes (write temp → fsync → rename).
- Manual and automated backup support with clear rotation strategy.
- Deterministic serialization order for world and state data.

### Usability
- Clear in-game feedback for core rules (engage range, resets, CC breaks).
- Companion control limited to turn and XZ movement for predictability.
- Simple server lifecycle: build → deploy → run as service.

### Compatibility & Upgrades
- Clear client/server version compatibility checks.
- Backward compatibility where feasible; explicit breaking changes.
- Changelogs focus on behavior and system-level changes.

## Use Cases

## Tasks Forge-Runner
- [x] tasks/forgecli_build_command.md
- [x] tasks/sms_llvm_aot_migration.md
- [ ] tasks/pose_library_and_retarget_workflow.md
- [ ] tasks/partial.md

fun main() {
    log.success("The answer of all questions is: ${getAnswer()}")
}

## Future Tasks
- [ ] Texture-Cache in `NodePropertyMapper` einbauen: `Dictionary<string, Texture2D>` nach resolved path, geteilt über alle Controls. Nur umsetzen wenn Ladezeiten > 1 Sekunde messbar sind.
- [ ] tasks/stackwide_single_slash_uri_scheme_policy.md
- [ ] tasks/resilient_source_distribution_playbook.md

## Tasks for Forge-Designer
- [ ] Lifecycle-Template für neue Scripts einführen (OnInit / OnReadyAsync sichtbar als optionaler Einstieg)
- [ ] Konventionsbasierte UI-Event-Hooks im Script-Template dokumentieren (z. B. treeItemSelected, treeItemToggled, listItemSelected)
- [ ] Script-Erzeugung im Designer auf neues Template umstellen

## Tasks for Forge-Poser
- [ ] tasks/ai_stylized_export_dialog.md
