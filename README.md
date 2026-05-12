# 🧭 The Forge 4D Vision  
Forge 4D is an open platform for building applications using SML (Simple Markup Language) and SMS scripting combining 2D UI, 3D scenes and animation timelines, powered by the Godot engine.

[![Manifest CI](https://img.shields.io/github/actions/workflow/status/CrowdWare/Forge/generate-manifest.yml?branch=main&label=manifest%20ci)](https://github.com/CrowdWare/Forge/actions/workflows/generate-manifest.yml) [![Release](https://img.shields.io/github/v/tag/CrowdWare/Forge?sort=semver&label=release)](https://github.com/CrowdWare/Forge/releases) [![License: GPLv3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://github.com/CrowdWare/Forge/blob/main/LICENSE)

**Native First. Write once, run anywhere – without the VM.**

Java had the idea in 1995. .NET followed in 2002. Both achieved portability through a virtual machine layer.
Forge takes the same goal further: one language, every platform, compiled natively via LLVM.
No garbage collector. No GC pauses. No VM overhead. Just your code, running directly on the hardware.

---

Forge 4D is not about avoiding code.  
It is about removing unnecessary barriers between ideas and reality.  

Today, creating software often means navigating complex toolchains, fragile frameworks, platform-specific quirks, and constant rewrites. This complexity excludes many people and slows down even experienced developers.  
Forge exists to change that.  

The name Forge 4D describes the environment itself.  
Forge can render traditional 2D user interfaces. Inside a viewport it can display interactive 3D scenes.  
When you add a timeline for animations and state changes, time becomes the fourth dimension for animation and interaction.  
2D + 3D + Time = Forge 4D.

Built with love, coffee, and a stubborn focus on simplicity.  

![screenprint](images/forge.png)

---

## 🧠 Core Idea

Describe what you want – not how the engine works.

Forge is built around a simple principle:

Structure and intent should be declared.
Execution and optimization belong to the runtime.

Instead of writing imperative glue code, creators describe UI, behavior, and structure using a simple, readable, declarative language (SML).
The Forge-Runner interprets this description and executes it on the target platform.

---

## 🚀 Getting Started

```bash
git clone https://github.com/CrowdWare/Forge.git
cd Forge
./setup.sh
```

`setup.sh` builds the CLI, installs `forgecli` to `~/.local/bin`, and adds it to your PATH — no sudo required.

Open a new terminal, then:

```bash
forgecli --help
```

### Build a tutorial app

```bash
# Mac desktop app
forgecli build mac --project tutorials/theme-demo-mac --output ./theme-demo-mac.zip

# Android APK
forgecli build android --project tutorials/theme-demo-android --output ./theme-demo.apk
```

### Prerequisites

- `cmake` (for building the CLI)
- `clang` (for SMS → native compilation)
- Godot 4 headless binary in PATH (for `build mac` / `build android`)

---

## 📌 Project Status

- Current Process: [CWUP/README.md](CWUP/README.md)
- Backlog: [BACKLOG.md](CWUP/BACKLOG.md)
- ForgePoser: [ForgePoser/README.md](ForgePoser/README.md)
- Documentation: [docs/](docs/)
- Tooling & generators: [tools/](tools/)
- Contribution model: [CONTRIBUTING.md](CONTRIBUTING.md)

### Related Tools

- **Forge STA (Speech To Action)** – voice driven workflow tool that converts speech into structured commands or prompts using Whisper.cpp and CLI post‑processing.  
  Speech → Whisper → CLI → structured output → application or AI pipeline.  
  Repository: https://codeberg.org/CrowdWare/ForgeSTA

This pipeline allows ideas to move from human expression (speech or text) into structured intent (SML/SMS) and finally into executable applications - without breaking the flow.

---

## 📜 Licensing

Forge uses dual licensing for source code:

- **Open Source:** GNU GPL v3 (or later), see [LICENSE](LICENSE)
- **Commercial:** available from CrowdWare for proprietary use, see [LicenseRef-CrowdWare-Commercial](LICENSES/LicenseRef-CrowdWare-Commercial.txt)
- **Guide:** practical distribution scenarios, see [COMMERCIAL.md](COMMERCIAL.md)

SML and SMS are open languages. You can freely write and use SML/SMS files for your own projects.

---

## 💡 Feature Requests & Feedback

If you have an idea, a workflow pain point, or a missing feature, please open an issue.

- Feature request: Use the repository's GitHub `Issues` tab and prefix the title with `[Feature]`.
- Bug report: Use the repository's GitHub `Issues` tab and prefix the title with `[Bug]`.

Clear, concrete examples are the fastest way to shape Forge in the right direction.

---

## Known Bugs (Blender Import)

When exporting posed/animated characters to `.glb` and opening them in Blender, the rig can appear as a
"golfball" style armature display. This is a Blender viewport/display issue around armature visualization.

See full guide: [docs/blender_armature_workaround.md](docs/blender_armature_workaround.md)

Short version:
- Hide `Armature` in Outliner, or set `Display As` to `Stick`/`Octahedral`.
- Use helper script: `tools/blender_armature_helper.py` (Mixamo + Meshy compatible).
  
---

## 📐 SML: Simple, Explicit, Honest

SML is intentionally minimal and means **S**imple **M**arkup **L**anguage.
	•	No hidden magic
	•	No complex expressions
	•	No JavaScript or JSON hacks
	•	No framework lock-in

SML focuses on:
	•	structure
	•	hierarchy
	•	intent
	•	readability

It is designed to be:
	•	written by humans
	•	generated by tools
	•	understood years later

If you can read it, you can reason about it.

Sample shows a minimal Forge application written in SML:  
```qml
Window {
	title: "My First App"
	pos: 0, 0
    size: 1920, 1080

	Label { text: "Hello World" }
}
```  
This file alone is enough to start a Forge application.  

---

## Core UI Components

Forge builds on the full UI system of the Godot engine.  
All standard Godot controls and dialogs are available, including buttons, panels, lists, trees, text inputs, menus and more.

Forge extends this foundation with additional components designed for building modern applications and tools.

### Workspace and Application UI

- **DockingHost** – main workspace container managing dockable panels  
- **DockingContainer** – dockable panels that can be rearranged by the user  
- **Window** – Forge window element for application layouts  
- **Markdown** – rich text and documentation viewer  
- **Terminal** – integrated terminal for command execution and logs  

### 3D and Time-Based Tools

- **Viewport3D** – embed interactive 3D scenes inside applications  
- **Timeline** – timeline component for animations and time-based workflows  
- **PosingEditor** – editor for adjusting poses and animation data  

These components make Forge suitable not only for traditional user interfaces,
but also for building complex tools such as editors, animation workflows,
and interactive 3D applications.

---

## ⚡ Event-First Scripting (No Dispatcher Boilerplate)
To be able to glue SML to the engine we created on very simple Kotlin based script language, we call **SMS** (**S**imple **M**ultiplatform **S**cript).
SMS is very easy to learn, because it only handles basic instructions.
```kotlin
fun ready() {
	var lang = os.getLocale()
    var msg = ""
    
    when(lang) {
        "de" -> msg = "Hallo Welt!"
        "es" -> msg = "¡Hola Mundo!"
        "fr" -> msg = "Bonjour le monde !"
        "pt" -> msg = "Olá Mundo!"
        else -> msg = "Hello World!"
    }
    log.info(msg)
}
```
SMS uses a direct event style:

`on <id>.<event>(...) { ... }`

That means no long dispatcher chains, no `if / else if` routing blocks, and no event-to-handler glue code you need to maintain manually.

Instead of this:
- receive generic event
- parse ids and event names
- route with if/else
- call business logic

you write this:

```sml
MenuBar {
    Menu {
        id: file
        title: "File"

        Item { id: saveAs text: "Save As..." }
    }
}
```

```sms
on saveAs.clicked() {
    log.info("Save As clicked")
    // your save-as flow here
}
```

This is one of Forge's strongest UX advantages:
- explicit and readable
- easy for newcomers
- deterministic for tooling/AI
- fast to evolve without architecture noise

---

## 🔒 Separation of Concerns (By Design)

Forge enforces a strict separation:
	•	Structure & UI → SML
	•	Behavior & Logic → native C# code
	•	Execution → Forge-Runner

This prevents the common collapse where UI, logic, and state become tangled and unmaintainable.

---

## 🧱 Deterministic, Portable, Durable

Forge systems are designed to be:
	•	deterministic
	•	platform-independent
	•	future-proof

SML files are plain text.
They can be versioned, diffed, reviewed, archived, and regenerated.

The same SML can be rendered:
	•	on Desktop
	•	on Mobile
	•	on Web
	•	inside game engines like Godot

without rewriting the description.

---

## 🛠 Forge Is Not a Toy

Forge is not a drag-and-drop toy for shallow demos.

It is a professional toolchain for:
	•	tools
	•	editors
	•	applications
	•	games
	•	books
	•	interactive content

Advanced users can extend the system with:
	•	native plugins
	•	WASM modules
	•	custom runners

Without breaking the simplicity of the core.

---

## 🤝 Empowering Humans, Not Replacing Them

Forge is not about replacing developers.

It is about:
	•	enabling designers to build real software
	•	allowing developers to focus on hard problems
	•	letting AI generate structure safely
	•	giving humans full control and ownership

AI can generate SML.
Humans can read it.
Runners execute it.

No black boxes.

---

## ⏳ A Long-Term Perspective

Forge is designed to survive trends.
	•	Frameworks will come and go
	•	Languages will change
	•	Platforms will disappear

But clear structure, explicit intent, and simple text formats endure.

Forge aims to be:
	•	boring in the best way
	•	stable
	•	understandable
	•	adaptable

---

## 🧩 In One Sentence

Forge is a human-first system for building software by describing intent, not fighting tools.
