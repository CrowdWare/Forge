# From Request to Production in One Push. No Mockup Tool Required.

*There is a rule in software development that everyone quotes and nobody questions:*
*"Don't use mockups in production."*

*That rule was written for paper prototypes. What happens when the mockup is the code?*

---

## The Traditional Workflow

A user has an idea. Here is what usually happens:

```
User request
  → Designer opens Figma
  → Designer builds wireframe (hours)
  → Review meeting (days)
  → Designer refines (hours)
  → Developer reads Figma, writes code (days)
  → QA (days)
  → Staging (days)
  → Production
```

Three to six weeks if you are lucky. The user who had the idea has forgotten
what they originally wanted.

---

## The Forge Workflow

A user has an idea. Here is what happens with Forge:

```
User request
  → Dev writes SML + SMS (minutes)
  → git push to Codeberg
  → User opens Forge app
  → User sees it live
  → "Yes, exactly that." or "Move the button left."
  → Dev adjusts, pushes again
  → Done.
```

No Figma license. No review meeting. No handoff document.
The first version the user sees is running code, not a picture of running code.

---

## What SML + SMS Looks Like

A user wants a screen with a counter and a reset button.

The developer writes this:

```sml
Column {
    padding: 24

    Label {
        id: counter
        text: "0"
        fontSize: 48
        color: "#1a1a2e"
    }

    Row {
        spacing: 16

        Button {
            id: btnIncrement
            text: "+"
        }

        Button {
            id: btnReset
            text: "Reset"
        }
    }
}
```

```sms
var count = 0

on btnIncrement.clicked() {
    count = count + 1
    counter.text = count
}

on btnReset.clicked() {
    count = 0
    counter.text = "0"
}
```

That is the entire app. Not a prototype. Not a wireframe.
This runs. Right now. On macOS, Android, anywhere Forge runs.

Push it to Codeberg. The user opens their Forge app, points it at the URL,
and sees a working counter with a reset button — in the time it takes
to make a coffee.

---

## "But Don't Use Mockups in Production"

Correct. Do not use mockups in production.

A mockup is a static image that pretends to be software.
It has no logic. It does not handle edge cases. It lies about behavior.

SML + SMS is not a mockup. It is compiled code.
The SML is parsed into a UI tree. The SMS compiles to LLVM IR.
The button actually increments the counter. The reset actually resets.

What looks like a sketch *is* the production artifact.

The reason "don't use mockups in production" exists is that
mockup tools produce outputs that cannot be deployed.
Forge produces outputs that cannot *not* be deployed.

---

## The Sandbox Is Not Optional

Every SMS script runs inside a sandbox.

The dev pushes to Codeberg. The user's Forge app downloads and executes that script.
This sounds dangerous until you read what the sandbox actually does:

- No filesystem access unless explicitly granted
- No network calls unless explicitly granted
- No shell execution, ever
- Stack depth limited (no infinite recursion attacks)
- Ahimsa policy: scripts declare peaceful intent or they do not run

The user verifies a working app. The app cannot reach outside its sandbox
without permission. This is not a security afterthought —
it is the architecture.

You can ship a script to a thousand users and none of them are exposed
to arbitrary code execution. The sandbox is always on.

---

## Staging Is One Push Away

Because the app is a URL, staging is trivial.

```
Codeberg: main branch    → production URL
Codeberg: staging branch → staging URL
```

User tests on staging URL. Approves. Dev merges.
Production updates automatically on next app open.

No deployment pipeline to configure. No Docker container to rebuild.
No environment variables to sync. The app *is* the SML and SMS files.
Change the files, change the app.

---

## Real Performance. Not Demo Performance.

We benchmarked this stack yesterday.

SMS compiled to LLVM IR on Apple M2, against C++ at the same optimizer level,
C# with JIT, and Kotlin JVM with JIT:

| Runtime | µs |
|---|---:|
| **SMS → LLVM IR** (no optimizer) | **71,358** |
| C++ `clang -O0` | 85,466 |
| C# .NET warm (JIT) | 96,656 |
| Kotlin JVM warm (JIT) | 64,445 |

SMS without any optimizer already outperforms C++ and C# JIT on compute workloads.

This is not demo performance. This is what runs when the user taps the button.

---

## The Full Loop

```
User:  "I want a counter with a reset button."

Dev:   [writes 20 lines of SML + 10 lines of SMS]
       [git push]

User:  [opens Forge app, navigates to URL]
       "Yes. Move the button to the right."

Dev:   [changes one line of SML]
       [git push]

User:  "Perfect. Ship it."

Dev:   [merges to main]

Done.
```

Total time: under 30 minutes.
Figma licenses used: zero.
Review meetings held: zero.
Users who forgot what they wanted: zero.

---

## What This Means for Teams

A developer can hand a Codeberg URL to a client at the end of a call.
Not a Figma link. Not a video recording. A live, interactive, sandboxed app.

The client can tap the buttons. They can break it. They can say
"this is not what I meant" — and the developer knows immediately,
not after three weeks of building the wrong thing.

The feedback loop that used to take weeks now fits inside one conversation.

---

## Try It

Forge is open source. The SML and SMS specifications are documented.
The sandbox policy is readable code, not a terms of service.

If you want to build something and show it to someone today —
not a picture of it, the actual thing —
Forge is the shortest path.

*Sat Nam. 🌱*

---

*Forge repository: [Codeberg](https://codeberg.org/CrowdWare/Forge)*  
*Yesterday's benchmark: `samples/bench_mac/run_bench.sh`*
