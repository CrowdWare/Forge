# Timeline – The Road to Forge

## Early Inspiration – Graphics and the Amiga Era
- Fascination with computer graphics began before learning software development
- Worked with **Deluxe Paint** on the **Amiga 500**, a legendary graphics tool for pixel art and game assets
- Received a book about Deluxe Paint as a Christmas present and spent the entire holiday period studying it intensely
- This period sparked a deep interest in **visual creation, graphics, and digital art**

## Graphic Design and Human‑Centered Design Influences
- Continued exploring graphics and visual tools over the years
- Later studied **Graphic Design**, including tools like **Adobe Illustrator**
- Worked extensively with **Adobe Image Styler**, a design tool that allowed creating styled graphics and UI elements
- Image Styler could generate visual styles and automatically render button states such as **Normal, Hover, and Pressed** as bitmaps
- Used it to experiment with visual styling, cubes, buttons, and interface elements
- This experience strongly influenced later thinking about **UI styling, visual systems, and design-driven application development**
- Motivation: building applications that look as beautiful and usable as early **iPhone apps**
- Strong influence from the design philosophy of Steve Jobs and Apple's focus on user experience
- Studied **Human‑Computer Interaction / Interaction Design**, combining software engineering with design principles
- Deeply influenced by the book **The Design of Everyday Things** by Don Norman
- One exercise from this design thinking later inspired the **Liquid Microcoins (LMC)** concept, including the idea of interacting with time through rotating clock hands to represent work minutes

## ~1987 (Age ~23) – First Real Programming
- 60‑hour C programming course
- First understanding of bits, bytes, and low‑level computing
- Started exploring Assembler and BIOS interrupts

## Shortly After – First Commercial Software
- Built a **Sprite Editor** in about 7 days
- Sold it to a company called Toolbox for **2000 DM**
- Realization: software can be created independently and even sold

## Dresdner Bank (Frankfurt) – First "Forge" Prototype
- Hired mainly to maintain an existing application
- Long waiting periods for tasks from the business department
- Used the time creatively to build a prototype system:
  - XML-based UI description
  - Parser that generated **Java Swing UIs**
  - Java interpreter written in Java used as **GUI glue**
- System was actually used in production
- Result: extremely fast **time‑to‑market** (bug fixes and changes sometimes deployed the next day)
- **The idea had no name yet. But it was already alive.**

## Webdesign Studies (SGD Darmstadt)
- Studied Webdesign to deepen understanding of web technologies
- Learned most required material within one month
- Briefly explored Adobe technologies but decided not to build on them

## Shift Toward Design
- Switched focus to **Graphic Design** studies
- Goal: create assets for applications and interfaces

## RIAMS – Silverlight Era
- Founded project/company **RIAMS (Rich Internet Applications & Multiplatform Services)**
- The idea already aimed at building services and tools that could support multiple platforms
- Attempted to build a **CMS / application platform** using:
  - Microsoft Silverlight
  - Telerik controls
- Conceptually similar to what Forge would later become

## Overload Period
At the same time:
- Full job (bank)
- Interaction Design studies
- Building RIAMS

Result:
- Severe overload
- Burnout

## Burnout and Turning Point
- Health crisis forced a stop
- Realization that working for the bank was not aligned with personal direction
- Corporate work abandoned

## Long Pause – Idea Dormant
- The original idea (application platform / UI system) did not disappear
- It simply waited for the right moment and technology

---

## The Detour That Wasn't a Detour
*This is the part most people skip in their story. They shouldn't.*

### Kotlin / Compose – The NoCode Designer Attempt
- Attempted to build a **NoCode Designer** using **Kotlin and Jetpack Compose**
- Gradle pulled in massive amounts of libraries and tools automatically
- Many dependencies were unknown: open source? alpha? stable? licensed how?
- No clear overview of what was actually in the project
- Licensing became a minefield: transitive GPL dependencies could silently break a dual-licensing model
- The complexity and lack of transparency became a blocker
- **The Gradle problem: you think you're building a project. Gradle thinks it's Christmas.**

### Game Development Exploration
- Parallel exploration of **game development** as a creative and technical outlet
- Tried **Unreal Engine** – too complex, too heavy for the intended purpose
- Tried **Unity** – commercial licensing concerns, ecosystem uncertainty
- Tried **Vulkan** – too low-level, too much boilerplate for application development
- Tried **Dear ImGUI** – functional but minimal, not suitable for rich application UIs

### The Godot Discovery
- Came across **Godot Engine** during the game development exploration
- Immediate observation: **Godot's UI system was significantly better than Compose**
- Better than Dear ImGUI
- Clean, node-based, visual, fast
- Key realization: *this is not just a game engine – this is the best UI framework I have ever worked with*
- Most developers look at Godot and see a game engine
- The insight here was different: **Godot is an application platform waiting to be used as one**
- MIT licensed – clean, permissive, no hidden traps
- This was not a planned decision. It was discovered through experience.
- **Intuition, guided by frustration, found the right tool.**

---

## ~28 Days Ago – Birth of Forge
- **February 10** – Started the **Forge** project
- Within the first days rapid progress happened due to iteration and AI-assisted development

### Early Milestone
- **Friday the 13th (February)** – First clear **USP milestone** reached
- Forge was already able to **render greybox scenes and send frames to Grok for style rendering**
- This proved the core concept: rapid scene creation in Forge followed by AI-based visual styling
- Marked the moment when the idea of Forge as a **previsualization / mockup pipeline for AI rendering** became tangible

Forge concept:
- **SML** – Simple Markup Language (UI structure)
- **SMS** – Simple Multiplatform Script (logic)

### The 28-Day Build
- SML Parser written in C# (Codex)
- SMS Parser and Interpreter written in C# (Codex)
- Full migration from 41,000 lines C# to 19,000 lines C++ 
- The GdScript Incident: Codex rebuilt the SML compiler in GdScript during C++ migration
- Claude brought in to fix architecture, clean up detour, complete C++ migration
- Grok integrated for image processing pipeline
- ForgeSTA created as a parallel tool: Speech → Whisper → structured output

---

## Evolution of the Poser Tool
The Poser did not appear suddenly. It is the latest step in a long evolution of tools for working with movement and animation.

### Sprite Editor (Early Years)
- One of the first tools created
- Built in about 7 days
- Focused on creating and editing sprites for games
- Sold for **2000 DM**, proving that independently built tools can have real value

### Animation Maker
- Built many years later as a **2D animation tool**
- Similar in concept to presentation tools or vector animation systems
- Allowed creation of **keyframe-based animations**
- Introduced ideas like timelines and animated transformations

### Poser
- The modern evolution of those earlier tools
- Focuses on **3D character posing and scene composition**
- Supports characters, bones, poses, and scenes
- Designed as part of the **Forge ecosystem**
- Represents the continuation of the original idea: building creative tools that enable rapid visual experimentation

---

## Today
Forge represents the **fourth incarnation** of an idea that started decades ago:

1. XML + Swing prototype at Dresdner Bank
2. Silverlight / RIAMS attempt
3. Kotlin / Compose NoCode Designer attempt
4. Forge – powered by Godot, built with an AI team

The idea survived.
The technology changed.
The intuition was right all along.

---

```
Forge {
    id: 1
    owner: Olaf Japp
}
```
