


# The Day I Realized What Is Possible With Forge

Sometimes a project grows slowly.

And sometimes there is a moment where everything suddenly clicks.

This was one of those moments.

---

## The Long Detour

Before arriving at the current architecture of Forge, I went on a long technical detour.

I explored several different paths:

- Unity
- Unreal Engine
- low‑level Vulkan development
- even the famous **"Hello Vulkan"** example with thousands of lines of setup code

All of these technologies are powerful. Extremely powerful.

But during this exploration something kept bothering me.

They are not designed for the problem I actually care about.

I don't want to build just another game.

I want to build a **platform for creating applications**.

A platform where ideas can quickly turn into working software.

---

## Returning to Godot

After this roundtrip through different engines and APIs, I returned to **Godot**.

And suddenly something became obvious.

Godot is not only a game engine.

It is actually a **very powerful application platform**.

It already provides:

- a modern rendering engine
- a full UI system
- animation
- video playback
- 2D and 3D scenes
- scripting
- cross‑platform export

In other words: a huge amount of infrastructure already exists.

Once I realized this, everything changed.

Forge suddenly made sense on top of it.

---

## Forge as a No‑Code Platform

Before this step, I experimented with a similar idea using **Jetpack Compose**.

Compose was great for building UI quickly.

But it is fundamentally limited to traditional application layouts.

Godot opened a completely different space.

Now applications can exist not only as windows, but also as **scenes**.

That means a UI element can exist:

- in a 2D interface
- inside a 3D environment
- projected onto objects

For example, you can render **a video onto a 3D wall** and still move the camera around the scene.

This kind of flexibility completely changes what an application can be.

---

## The GreyBox to Stylized Video Pipeline

Another experiment that suddenly became possible is something I call:

**GreyBox → Stylized Video**.

The idea is simple.

Start with a greybox scene:

- simple characters
- basic animation
- no textures
- no assets

Then export that scene and send it through an AI styling pipeline.

The result is a **stylized video output** that visualizes the scene without requiring a full production pipeline.

In practice this looks like:

GreyBox Scene  
↓  
Animation  
↓  
Forge CLI  
↓  
AI styling (Grok)  
↓  
Stylized Video

Seeing this work for the first time was a moment where I literally thought:

> Wow. This opens a lot of possibilities.

---

## What This Enables

Once you have a system like this, many use cases suddenly appear:

- application previews
- UI mockups
- game scene previews
- architectural visualization
- animated storyboards
- concept videos for ideas

Instead of building full assets, you can quickly generate **visual prototypes**.

And that changes how fast ideas can be communicated.

---

## Why Forge Is Open

When I realized what was possible, one thought immediately followed: This should not remain a private experiment.
That's why Forge is being developed in the open - with open technology, open experimentation, and open discussion.
Forge itself is and will remain fully open source. The licensing model reflects this spirit: open source projects built with Forge are completely free. If Forge is used inside a commercial closed-source product, a commercial license is required.
The goal is an ecosystem where knowledge stays shared - and where **commercial use helps fund the people building and maintaining the technology**.

---

## An Experiment Worth Sharing

Forge is still evolving.

But that moment of realization - seeing how these pieces fit together - made one thing clear to me.

We are only beginning to explore what is possible when:

- structured scene descriptions
- application runtimes
- and AI pipelines

start to work together.

Forge is my attempt to explore that space.

---

## Follow the Journey

Forge is being built in public.

If you are curious about where this experiment goes next, you can follow the project here:

https://codeberg.org/CrowdWare

Ideas, feedback, and experiments are always welcome.