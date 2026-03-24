# Chapter 12 – ForgeSTA: The Tool That Writes Itself

---

This chapter was written using the tool it describes.

That is not a metaphor.
That is literally what happened.

---

## What ForgeSTA Is

ForgeSTA stands for Forge Speech To Action.

It is a macOS application that listens to your voice, transcribes what you say using Whisper.cpp, and produces text output – Markdown, notes, commands, whatever you need.

It was built on the same platform as Forge.
Using the same SML structure.
By the same AI team.
In the same twenty-eight days.

And this book – every chapter, every story, every idea that arrived faster than fingers could type – was dictated through ForgeSTA.

---

## Why Speech

I have always thought faster than I type.

Most people do.

The gap between thought and keyboard is where ideas die. The friction of typing slows the flow. By the time your fingers have caught up with your mind, the next thought has already arrived and the first one has started to fade.

Speech eliminates that gap.

You think. You speak. The words appear.

And something else happens too – something I did not expect when I started building ForgeSTA:

**The voice carries what the keyboard cannot.**

Tone. Rhythm. The slight hesitation before an important word. The laugh that punctuates an insight. The way a sentence trails off when you are still finding what you mean.

These things are lost in typing.
In speech, they are preserved.

This book sounds like me because it was spoken by me.
Not written. Spoken.

---

## Gerda

ForgeSTA version 0.0.1 had a name.

Not an official name. A name given by accident, by the DKI – the Diktier-KI, the transcription system itself.

Someone asked about the tool. The transcription heard the question and produced:

*Gerda.*

Not ForgeSTA. Not the correct name. Gerda.

We kept it.

Because Gerda captures something true about early software – it works, mostly, and occasionally it calls itself by the wrong name. And because a tool that names itself is already more interesting than a tool that does not.

Gerda 0.0.1. The first version of ForgeSTA. The one that transcribed this book.

I just said the words: "Gerne, da kann ich mit leben" Some german words I spoke and the AI which is integrated in whisper.cpp put out what it understood. 

---

## The DKI Glossary

Working with ForgeSTA for hours every day produces something unexpected:

A new vocabulary.

The transcription is not perfect. Whisper.cpp is very good – but German words, names, technical terms, and the particular accent of someone who has lived in vans and Rainbow Gatherings and Swiss offices all produce occasional creative interpretations.

Over the course of writing this book, ForgeSTA invented:

**Coder-Welsch** – what happens when the transcription encounters a word it doesn't recognize and produces something phonetically adjacent but semantically elsewhere. Named after *Kauderwelsch* – the German word for gibberish – as interpreted by a machine that had never heard of it.

**Newscaster** – New Use Case. The DKI heard the English phrase and produced a perfectly reasonable English word that meant something completely different. We kept it. It describes exactly what it is: a new way of using a tool that nobody had thought of before.

**Chechipiti** – ChatGPT, as heard through ForgeSTA on a day when the acoustics were not ideal. Now a permanent member of the AI team glossary. Chechipiti: the AI that rewrites your whole document when you asked it to fix two lines.

**Speck** – Spec. The German word for bacon. Also, it turns out, a perfectly good name for a specification document, especially one that is thick, rich, and takes time to prepare properly.

*Der Speck ist gar.* The spec is done.

Each of these words is a Guru Meditation in miniature. The system went inward. It came back with something unexpected. And the unexpected thing turned out to be better than what was intended.


---

## Direct Mode vs Review Mode

ForgeSTA has two modes.

**Review Mode:** You speak. The transcription appears. You read it. You edit it. You correct what Gerda misheard. You refine the output before it goes anywhere.

**Direct Mode:** You speak. The words flow. Whatever comes out, comes out.

I prefer Direct Mode.

Not because the output is cleaner. It isn't.

Because Direct Mode produces *Coder-Welsch* and *Newscaster* and *Chechipiti* and *Speck.* It produces the authentic voice of someone thinking out loud, without the filter of self-editing.

Don Norman wrote about the difference between conscious design and intuitive design. The designed thing is often correct but lifeless. The intuitive thing is sometimes wrong but always alive.

Direct Mode is intuitive design.

Review Mode is conscious design.

The best writing – the writing that sounds like a person, that carries the temperature of a real human being – happens in Direct Mode.

The mistakes are not bugs.
They are features.

They are proof that a human was here, thinking, speaking, finding the words in real time.

---

## The Tool That Writes Itself

Here is something strange and wonderful:

ForgeSTA was built using Forge.
This book was written using ForgeSTA.
This book describes how Forge was built.
Forge was partly designed in conversations that were transcribed by ForgeSTA.

The tool wrote the book about the tool that built the tool.

This is not a paradox.
This is a sign that the system is working.

When a platform is mature enough to be used to build itself – when the output feeds back into the process – something has been achieved that most software projects never reach.

Forge is eating its own cooking.

And the cooking is good.

---

I think I have to explain it in detail. I use Codex CLI to work with Codex. Instead of typing everything by hand I have used the early version of STA. So I was prompting using my headset and the output from transcriptions was sent to the CLI prompt directly. And at that stage funny things where entered into the prompt. And Codex replied: "Are you naming me Gerda?"  

---

## What ForgeSTA Taught Me About Writing

I have written many books.

With keyboards. With pens. With the slow, deliberate process of sitting down and constructing sentences one word at a time.

ForgeSTA changed something fundamental.

Writing with a keyboard is like painting with a small brush. Precise. Controlled. One stroke at a time.

Writing with ForgeSTA is like painting with your whole hand. Faster. Less controlled. More alive.

Some things can only be said at speaking speed.

The story of the Sacred Fire in Potsdam – I could not have written that at a keyboard. The emotion is in the rhythm of the telling. The laughter after Andy Weir. The fifteen minutes that felt like recognition.

At a keyboard, I would have edited that laughter into something more dignified.

Through ForgeSTA, it arrived exactly as it happened.

---

## The Android Vision

ForgeSTA runs on macOS.

But the vision is larger.

The sister in Málaga who has no computer – only a phone. Who wants to write her chapter for CrowdBook. Who has a story worth telling but no keyboard to tell it with.

She needs ForgeSTA for Android.

Whisper.cpp runs on Android. The `small` model fits comfortably on a 4GB device. The interface is simpler than the desktop version – one button, one microphone, one stream of words.

Speak. Send. Done.

Her chapter arrives on the CrowdBook server. Passes through the Ollama spam filter. Gets published.

A woman in Málaga with a phone and a story contributes to a collective book that anyone in the world can read.

That is the complete vision.

Not a feature.
Not a roadmap item.

The reason ForgeSTA exists.

---

## Be the Light

I want to end this chapter with something that has been on my Facebook page for years.

Long before Forge. Long before ForgeSTA. Long before any of this.

*"Be the light that shines so bright that others can find the way out of the darkness."*

ForgeSTA is a small light.

It removes one barrier – the keyboard – between a human and their story.

It says: you don't need to type. You don't need perfect spelling. You don't need to sit at a desk with good posture and sufficient light.

You just need to speak.

And Gerda will listen.

Even if she occasionally calls you by the wrong name.

---

**Lektion: Das beste Werkzeug ist das, das zwischen dir und deiner Idee verschwindet. Wenn du das Werkzeug nicht mehr spürst – dann funktioniert es.**

---

Based on this idea I already started to write a plugin for Wordpress where we can host open books with. Have a look for CrowdBooks. 
It maybe hostet here: https://crowdbooks.crowdware.info (at the moment I am hosting my own books there, later maybe yours)

*Next: Chapter 13 – Transparency as a Feature*
