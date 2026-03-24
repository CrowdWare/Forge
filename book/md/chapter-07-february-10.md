# Chapter 7 – February 10: The Third Attempt Begins

---

The idea doesn't come when you are looking for it.
It comes when you are watching something else entirely.

---

## The Raid Simulator

It started with World of Warcraft.

Not the whole game. Just the raids.

If you have never played WoW, here is what you need to know: a raid is a coordinated attack on a powerful enemy, requiring ten to forty players moving in perfect synchronization. Positioning. Timing. Roles. Communication. It is, in its own way, a kind of art.

I didn't want to build the whole game. The grinding. The leveling. The endless quests. Just the raids. A simulator. A tool for people who love the tactical heart of WoW without everything around it.

I am, after all, a tool builder.

So I started looking for a way to build dungeons. Environments. Spaces where the simulation could live.

This is where the labyrinth began.

---

## Godot, First Visit

Godot is an open source game engine. MIT license. Free forever. No royalties. No corporate overlord deciding what you can and cannot build with it.

For these reasons alone, Godot was the right choice philosophically.

We started building. Codex wrote code. I directed. We made progress.

And then we hit a wall.

Something Godot couldn't do – or couldn't do the way I needed it done. I don't remember the exact technical detail. I remember the feeling:

*This tool is wrong. Next.*

When you have been building software for thirty years, you develop a sensitivity to this moment. The moment when fighting the tool costs more than finding a different tool. The moment when the path forward is not through but around.

I said: Godot is bad.

This was not entirely fair to Godot.
But it was the right decision.

---

## The Vulkan Detour

Someone suggested Vulkan.

Vulkan is a graphics API – low level, extremely powerful, extremely demanding. The kind of tool that gives you complete control over the GPU and expects you to know exactly what to do with that control.

I looked at Hello Vulkan – the simplest possible Vulkan program. The equivalent of printing "Hello World."

It was 1,800 lines of code.

My mind said: too complex. Walk away.

My stubbornness said: I have AI. The AI will write it.

So we went down the Vulkan path. Codex wrote boilerplate. We integrated a GUI layer described in SML – because of course we did, SML was already in my head by then. We made progress.

And then something shifted.

Not a technical failure. Not a wall. Something quieter.

A feeling that said: *this is too much code. Too much complexity. Too much to maintain. This is not the right shape for what you are building. And then Jolt arrived. A physics engine we needed, an ABI we couldn't resolve. The universe had made the decision for us.*"

I have learned to trust this feeling.

Even when I cannot explain it.
Even when the code is working.
Even when walking away feels like failure.

I walked away from Vulkan.

---

## Back to Godot

And then I watched a YouTube video.

I don't remember who made it. I don't remember the title. I remember only what it showed:

Someone using Godot to build a normal application.

Not a game. Not a 3D world. Not a simulation.

A regular desktop application. With panels and buttons and menus and all the ordinary furniture of software that people use every day.

Using Godot's GUI controls. As an application framework.

I watched this and something happened that I can only describe as:

*Click.*

Not metaphorically. Literally – something in my mind clicked into place. A connection formed between things that had been sitting separately, waiting for exactly this moment to come together.

Godot's GUI controls are excellent.
SML can declare structure.
If SML declares the structure and Godot renders it –

Then you have a platform.

Not just for desktop apps.
For mobile. For web. For anything Godot can target.
And Godot can target almost everything.

And it's better than Jetpack Compose because Godot can do video.
And 3D.
And games.
And interactive content.
And books.

I saw, in that moment, not just a solution to one problem but solutions to problems I had been carrying for years. Problems from the Dresdner Bank prototype. Problems from the Kotlin attempt. Problems from every version of this idea that had come before.

*This solves this. And this. And this. And this.*

The pieces had been waiting.
The YouTube video assembled them.

---

## The Language Question

We started building in C#.

C# is one of my favourite languages. Ten years in Switzerland, building everything that needed building, in C#. I know it well. I know its patterns, its idioms, its strengths.

But something bothered me.

Every time the code compiled, somewhere in the toolchain, a word appeared:

*Microsoft.*

And I thought about what Microsoft represents. The proprietary ecosystem. The Windows-first thinking. The embrace-extend-extinguish history. The world where your tools belong to a corporation that can change the rules whenever it suits them.

Mono existed as an escape route. But Mono felt like a workaround, not a solution.

I wanted something clean.
Something that belonged to no one.
Something that would still compile in twenty years regardless of what Microsoft decided.

So I considered Rust.

Rust is memory-safe by design. Modern. Fast. The language everyone says is the future. I thought about it seriously for a while.

And then I thought about what we were actually building.

The C++ code – the Forge Runner – would be generated by AI. Written once. Reviewed for bugs. Fixed when broken. But never rewritten from scratch. Never evolved feature by feature by a human typing every line.

It would sit there. Doing its job. Invisible.

And for that purpose – for generated, stable, permanent infrastructure code – C++ made more sense than anything else.

Fast. Portable. Proven by decades. And with RAII – Resource Acquisition Is Initialization – the memory management that Rust was invented to replace already exists, handled correctly by the compiler, in code that Codex knows how to write.

We would never touch this code again in the normal sense.
We would review it. Fix bugs. But not evolve it by hand.

It was infrastructure.
Infrastructure should be boring.
C++ is boring in exactly the right way.

*C++ it is.*

---

## February 10

I don't remember a dramatic moment.

No thunderclap. No vision. No voice from above.

Just: February 10, 2026. A decision that had been forming for months – through the Raid Simulator, through Godot, through Vulkan, through the YouTube video, through the language deliberations – finally became action.

*Third attempt. This time we finish it.*

I opened a new project.
I called it Forge.
I told Codex what we were building.

And we began.

---

## Why Third

The first attempt was at Dresdner Bank. XML and Swing. The prototype that worked but lived in a bank and couldn't leave.

The second attempt was Kotlin and Jetpack Compose. The right idea, the wrong tools. The attempt that taught me what the platform needed to be before it could exist.

The third attempt was this.

Forge. Godot. C++. SML. SMS. An AI team of three.

Each attempt had failed in exactly the right way. Each failure had taught me something the next attempt needed. Each dead end had been a navigation tool in disguise.

The first attempt said: *the idea works, but the platform is wrong.*
The second attempt said: *the platform is closer, but the tools are wrong.*
The third attempt said: *the platform is right, the tools are right, the team is right.*

This is not perseverance.
This is not stubbornness.

This is the difference between giving up on an idea and giving up on an approach.

The idea never failed.
Only the approaches failed.
And each failure was a gift.

Guru Meditation #00000003.

The system went inward.
The system came back.
The system knew more than it did before.

---

## 28 Days

From February 10 to March 13 (today).

Twenty-eight days plus a few days documenting.

In that time, with Codex writing C# and then C++, with Claude designing architecture, with Grok handling image processing, with ForgeSTA transcribing ideas that arrived faster than fingers could type:

Forge 4D became real.

Not finished. Software is never finished.
But real. Runnable. Demonstrable. True.

After these twenty-eight days, Codex measured what we had built:

C++ (.cpp): 19,003 lines – 55.0%
GDScript (.gd): 4,526 lines – 13.1%
C# (.cs): 4,216 lines – 12.2%
SMS (.sms): 2,934 lines – 8.5%
SML (.sml): 1,923 lines – 5.6%
C/C++ Header (.h): 1,010 lines – 2.9%
Shell (.sh): 731 lines – 2.1%
Python (.py): 233 lines – 0.7%

Total: 34,576 lines of code.

C++ is the foundation – the Forge Runner, invisible and permanent. GDScript exists only to generate documentation and code directly from the Godot database. A CLI for Grok access, written in C#, still waiting for its moment.

Nobody typed most of this.
It was generated, directed, reviewed.
By a human who knew what he wanted.
And three AIs who knew how to build it.
An SML parser that reads structure and renders it.
An SMS interpreter that handles behavior.
A runner that works on desktop, with mobile and web in progress.
We deleted 41,000 lines of C#.
Without regret.
Because the idea was right. The language was wrong.

And a book, being written in the tool that the tool makes possible.

Twenty-eight days.

Because the idea had been waiting for thirty years.
And when everything was finally ready –

It didn't need long.

---

**Lektion: Die Idee scheitert nie. Nur der Ansatz scheitert. Und jedes Scheitern ist eine Navigationshilfe.**

---

*Next: Chapter 8 – Codex: The Code Writer*
