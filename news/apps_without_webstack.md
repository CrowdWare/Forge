# Apps Without the Web Stack

## The Idea Behind Forge

The idea did not start with Forge.

It started much earlier.

Back when Microsoft introduced **HTA – Hypertext Applications**.
A simple idea:

> Use HTML, but without a browser.

An application running directly on the desktop.

At the time, this felt like a small revelation to me.

Not because HTA was perfect - but because it raised an important question:

**Why do applications need a browser at all?**

---

## The Browserization of Software

Many years later the world looks like this:

Electron  
WebViews  
Huge JavaScript frameworks  
Gigabytes of Node dependencies

Today we build desktop apps by…

…starting a browser.

For many things this works.
But as a developer it often feels wrong.

Too heavy.  
Too slow.  
Too many layers.

---

## The First Version

The idea never completely left me.

During my time at **Dresdner Bank** I once sat in support waiting for a system to stabilize.

A department had a typical request:

> “We need a solution tomorrow.”

Time‑to‑market.

Developing new software usually took weeks or months.

So I started experimenting.

Instead of:

HTML  
JavaScript  
Browser

I built a small platform based on **Java Swing**.

The UI was described in **XML**.  
The logic ran using **JavaScript inside the JVM**.

In many ways it was an early prototype of what Forge is today.

Applications could be launched **almost instantly**.

The system worked surprisingly well.

I even built a small editor for it.

---

## Confirmation

Over the years similar ideas kept appearing.

Mozilla experimented with **XUL**.

Microsoft experimented with **Silverlight**.

Most of these approaches eventually disappeared.

But they confirmed something for me:

> The idea itself was not wrong.

It was simply ahead of its time.

---

## Then AI Arrived

Many years later I started experimenting again.

This time the situation was different.

AI can now generate large amounts of code.

With tools like **Claude** and **Codex**, an idea can quickly turn into tens of thousands of lines of working code.

That changed everything.

A platform that would once have taken years to build alone suddenly became realistic.

---

## Forge

Forge is the result of that long journey.

The idea is simple:

**Apps without the web stack.**

The user interface is defined in **SML (Simple Markup Language)**.  
Application logic runs in **SMS scripts**.

No browser runtime.

The renderer is based on **Godot**, an open‑source engine.

This means:

- native performance
- modern UI capabilities
- 2D and 3D rendering
- video and animation
- sandboxed scripting

An application can even be projected onto a **3D surface** - for example onto a wall inside a scene.

---

## Security

A key aspect of Forge is the **sandbox**.

Scripts do not run directly on the host system.

They run inside a controlled environment.

This is especially important for platforms such as:

- iOS
- iPadOS

Apple does not allow applications that execute uncontrolled external code.

Forge therefore aims to provide:

> A secure scripting runtime inside a native application.

---

## Why Build This?

Because modern software has become unnecessarily complicated.

Many applications now consist of:

Browser  
Framework  
Framework on top of framework  
Build toolchains  
Node modules  
Bundle systems

Forge explores a different direction.

Not:

Web apps everywhere.

But instead:

> A platform that builds applications directly from structured ideas.

In the long run the workflow could even look like this:

Speech → Structure → Application

---

## An Experiment

Forge is still young.

But it already exists:

- hundreds of commits
- tens of thousands of lines of code
- its own scripting and markup languages

Forge is an experiment.

The question is simple:

> Can we make applications simple again?

Without browsers.  
Without massive toolchains.

Just:

**Apps without the web stack.**