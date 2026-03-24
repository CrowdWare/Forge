# Chapter 8 – Codex: The Code Writer

---

A good team member does what you ask.
A great team member does what you mean.
Codex does both.
Sometimes in the wrong language.
But always with complete commitment.

---

## The Third Team Member

By February 10, I had assembled a team.

Not in the traditional sense. No office. No contracts. No stand-up meetings on Monday mornings where everyone says they are fine when they are not.

Three AI systems, each with a distinct role:

**Codex** – writes code.
**Claude** – designs architecture and thinks.
**Grok** – handles specialized tasks like image processing.

And me – the human in the driver's seat. The one who knows where we are going, even when the GPS suggests a different route.

This chapter is about Codex.

---

## What Codex Does

Codex writes code the way a river fills a valley.

You show it the shape of the land – the architecture, the interfaces, the expected behavior – and it fills the space. Completely. Without complaint. Without asking why.

Give Codex a specification and it returns working code.
Give it a bug and it returns a fix.
Give it a direction and it returns a path.

It does not have opinions about whether the architecture is elegant.
It does not have feelings about deleted work.
It does not mourn the 41,000 lines of C# we deleted without ceremony.

It simply builds what is asked.

This is not a limitation.
This is exactly the right tool for exactly this job.

---

## The Numbers

Let me show you what Codex built in twenty-eight days.

Not estimates. Not approximations. Measured:

| Language | Lines | Share |
|----------|-------|-------|
| C++ (.cpp) | 19,003 | 55.0% |
| GDScript (.gd) | 4,526 | 13.1% |
| C# (.cs) | 4,216 | 12.2% |
| SMS (.sms) | 2,934 | 8.5% |
| SML (.sml) | 1,923 | 5.6% |
| C/C++ Header (.h) | 1,010 | 2.9% |
| Shell (.sh) | 731 | 2.1% |
| Python (.py) | 233 | 0.7% |
| **Total** | **34,576** | **100%** |

C++ is the foundation – the Forge Runner, invisible and permanent, doing its work without asking for attention. GDScript exists only to generate documentation and code directly from the Godot database. A CLI for Grok access, written in C#, waiting in the wings.

Nobody typed most of this.

It was directed. Generated. Reviewed. Corrected when wrong. Accepted when right.

By a human who knew what he wanted and three AIs who knew how to build it.

---

## The 41,000 Lines We Deleted

Before these 34,576 lines, there were others.

In the early phase of Forge, before the architecture was fully clear, Codex generated approximately 41,000 lines of C#. A substantial codebase. Months of work by traditional standards.

We deleted it.

Not because it was bad code. Because the language was wrong. Because when I looked at the Microsoft logo in the toolchain and thought about proprietary ecosystems and Windows-first thinking, I knew we needed to go further.

C++ was the answer. C# was the learning.

And here is the thing about deleting code that Codex wrote:

It doesn't hurt.

When you have written every line yourself – when each function represents an hour of your thinking, each class a day of your life – deletion feels like loss. You hesitate. You create backup folders named "old_version_maybe_useful_later." You carry dead code for years because letting go is painful.

When Codex wrote it, you delete it like you delete a draft email.

*This no longer serves the direction. Remove.*

This is one of the hidden advantages of working with AI that nobody talks about:

**You become unattached to the code.**

And unattachment to code is one of the most important qualities a software architect can have.

Code is not the product.
The idea is the product.
Code is just the current best expression of it.

---

## Vibe Coding

There is a term for what we were doing: Vibe Coding.

Not writing specifications first. Not designing UML diagrams. Not planning every interface before a single line is written.

Just: describing what you want. Iterating. Feeling the direction. Following the pull.

*I want a parser that reads SML and builds a scene graph.*
*I want a script interpreter that handles basic control flow.*
*I want a GUI layer that maps SML nodes to Godot controls.*

Codex fills in the how.
I provide the what and the why.

This is not lazy development.
This is the correct division of labor between human and machine.

Humans are good at vision. At knowing what matters. At feeling when something is wrong before they can articulate why.

Machines are good at implementation. At filling structures. At not getting tired at 2am.

Together, we are faster than either alone.
And more importantly – we are building the right thing.
Because the human never left the driver's seat.

---

## What Codex Cannot Do

Codex cannot decide what to build.

This sounds obvious. It is not obvious to everyone.

I have watched developers hand their entire project to an AI and ask it to decide the architecture. To choose the approach. To determine what matters.

The AI complies. It produces something. Often something that looks correct.

But it is building without a destination. Without the vision that only comes from a human who has been carrying an idea for thirty years and knows – in their body, not just their mind – what it needs to be.

Codex is extraordinary at implementation.
It is useless at intention.

Intention is mine.
Implementation is Codex.

This boundary, kept clearly, is what made Forge possible in twenty-eight days.

---

## Iterating Without Fear

In twenty-eight days we went through more versions than I can count.

Ten versions of the SML parser.
Multiple rewrites of the SMS interpreter.
Architecture changes that would have taken weeks in traditional development.

With Codex, a rewrite is an afternoon.
With Codex, an experiment costs nothing but time.
With Codex, the question is never *can we afford to try this?*

The question is always: *is this the right direction?*

And that question – the only question that actually matters – is answered by the human.

By intuition.
By thirty years of building things and knowing what works.
By the same feeling that recognized Deluxe Paint before the manual arrived.

---

## The Contributor

Somewhere in the Forge repository, in the contributor list, there is an entry.

Not Artanidos. Not a human name.

An AI-generated commit message. A contribution from a system that wrote tens of thousands of lines without asking for credit.

I left it there deliberately.

Because honesty requires it.
Because the work was real.
Because in a world where AI contribution is hidden and humans take sole credit for machine-assisted work, I wanted to be different.

Forge was built by a team.
The team included AI.
The repository says so.

This is not humility.
This is accuracy.

---

## Trust, But Verify

Codex is not infallible.

There was the GdScript Incident – which has its own chapter, because it deserves one. There were bugs. There were misunderstandings. There were moments when Codex solved the right problem in the wrong language, or the wrong problem in the right language, or produced code that compiled perfectly and did something entirely different from what was intended.

This is why the human never leaves the driver's seat.

Not to micromanage.
Not to rewrite what Codex writes.
But to verify. To test. To say: *this is not what I meant, try again.*

The GPS metaphor is useful here:

You tell the GPS your destination.
The GPS finds a route.
You follow the route – but you look out the window.
You notice when the GPS is taking you somewhere wrong.
You override when necessary.

You do not let the GPS drive.

Codex is the GPS.
The destination is mine.
The override is always available.

---

**Lektion: KI schreibt den Code. Du schreibst die Intention. Verwechsle niemals die beiden.**

---

*Next: Chapter 9 – The GdScript Incident*
