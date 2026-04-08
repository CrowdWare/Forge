# Forge OS - Konzept

> *"Amiga 500 Feeling - ohne Bootviren"*
> *"Compile once, runs 10 years"*

## Die Idee

Forge OS ist kein weiteres Linux-Derivat. Es ist eine neue Schicht **über** dem Linux-Kernel - eine schlanke, ehrliche Plattform für alte und neue Hardware, gebaut auf der Philosophie: **Software gehört allen. Hardware stirbt nicht. Wissen ist frei.**

Inspiriert von Ubuntu (dem afrikanischen Prinzip: *"Ich bin, weil wir sind"*), der Rainbow Family und dem Geist des Amiga 500.

---

## Zwei Flavors - ein Kernel

### Forge OS 500
- Amiga 500 Ästhetik
- GUI via SML + SMS
- Godot 2D Renderer
- Für alte PCs ab ~512MB RAM

### Forge DOS
- MS-DOS / Turbo Pascal Ästhetik
- TUI Textmodus Renderer
- Selbe SML/SMS Apps - nur anders gerendert
- Für noch ältere Hardware

**Beide basieren auf demselben Linux-Kernel. Derselbe App-Code läuft auf beiden.**

---

## Der Stack

```
Linux Kernel (C - unverändert, bewährt)
        │
        ├── Forge Runner (C++ / Godot)
        │       └── SML/SMS Apps (UI)
        │
        └── Go Services (HTTP/REST)
                ├── Core Services
                └── Community Services (erweiterbar)
```

### Jede Sprache hat genau einen Job

| Sprache | Aufgabe |
|--------|---------|
| C | Kernel, Hardware - unantastbar |
| C++ | Forge Runner, Godot Renderer |
| Go | Services, HTTP/REST, Middleware |
| SML | UI Layout |
| SMS | UI Logik, App-Verhalten |

---

## App-Struktur

Jede Forge-App ist ein selbst-contained Paket:

```
MyApp/
├── ui/
│   ├── app.sml        - UI Layout
│   └── app.sms        - UI Logik
├── backend/
│   └── service.go     - Middleware / Business Logik
└── data/
    └── (SQLite)       - lokale Datenbank
```

- Bringt ihr eigenes Backend mit
- Bringt ihre eigene Datenbank mit
- Keine externen Abhängigkeiten

---

## IPC - Interprozesskommunikation

Apps kommunizieren über **HTTP/REST auf localhost**:

```
localhost:8000/app/{UID}/action
localhost:8000/app/{UID}/event
```

- Ein zentraler Forge Runner Port
- Jede App registriert sich mit einer UID
- Forge Runner = zentraler Message-Router
- Debugging trivial - einfach curl

---

## Was NICHT drauf läuft

Das ist kein Mangel - es ist ein **Design-Prinzip**:

- ❌ Electron
- ❌ Browser
- ❌ Node.js
- ❌ Java
- ❌ Swift
- ❌ Fremde C++ Apps

Was drauf läuft:
- ✅ Forge Runner
- ✅ SML/SMS Apps
- ✅ Go Services (als Binaries)
- ✅ Das wars

---

## Datenbank

**SQLite** als Standard - eine Datei pro App:
- Kein Datenbankserver nötig
- Läuft auf alter Hardware problemlos
- WAL-Mode für parallele Zugriffe
- Passt zu "compile once, runs 10 years"

---

## Bootviren - das killer Feature

Community-Mitglieder können **"Viren"** bauen:

- Machen nichts kaputt - vollständig sandboxed
- Sind einfach Forge-Apps mit dramatischem Intro
- Verbreiten sich durch Teilen
- Können den User auffordern selbst etwas einzugeben 😄

```
DRINGEND SYSTEMWARNUNG!!!
Guru Meditation #00000404.PAGE_NOT_FOUND
- eingefroren, hilft nix -
```

**Bootviren als Content-Mechanik:**
- Community baut Viren = Community baut Apps
- Virales Marketing im wörtlichsten Sinne
- Security-Awareness als Spaß verpackt

---

## Zielgruppe

- Amiga-Nostalgiker (40–55 Jahre)
- Retro-Linux-Nerds
- Sustainability-Bewusste - alte Hardware weiterverwenden
- Junge Leute mit Retro-Ästhetik-Affinität
- Menschen in Ländern mit älterer Hardware-Infrastruktur

**Hardware-Empfehlung:** Geräte für 15–30€ auf Kleinanzeigen.de

---

## Die Message-Schichten

```
Oberfläche:      "Haha Bootviren und Amiga-Look - cool!"
        ↓
Zweite Schicht:  "Warte, das läuft auf meinem 20€ PC?"
        ↓
Dritte Schicht:  "Niemand verdient daran? Alles offen?"
        ↓
Tiefste Schicht: Ubuntu-Philosophie, OFC, Sharing Economy
```

Menschen kommen wegen dem Spaß - bleiben wegen der Philosophie.

---

## Philosophische Basis

Drei Philosophien - ein Prinzip:

| Tradition | Ausdruck |
|-----------|---------|
| Ubuntu (Afrika) | Ich bin, weil wir sind |
| OFC | Offene freie Zusammenarbeit |
| Rainbow Family | Jeder bringt mit was er kann |

---

## Lizenz

**libera** - frei wie in Freiheit.

---

## Status

💡 Konzept-Phase - Ideen willkommen!

Forge OS entsteht als Teil des [Forge 4D](https://codeberg.org) Projekts.
SML + SMS - zwei Textdateien. Eine App. Läuft überall.

---

*"Wir bauen kein OS für euch - wir bauen es zusammen."*
