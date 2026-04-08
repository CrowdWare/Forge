# Forge Markdown Editor - Spec v1

*Ziel: Bücher schreiben auf Android Tablet, mit Git-Anbindung zu Codeberg*

---

## Use Case

Art fährt über Ostern weg.
Tablet im Gepäck.
Keine Ablenkung. Nur Text.

Schreiben → Speichern → Pushen → fertig.

---

## Die App

### UI: Split-View

```
┌─────────────────┬─────────────────┐
│  EDITOR         │  PREVIEW        │
│                 │                 │
│  # Kapitel 1    │  Kapitel 1      │
│  Das ist Text   │  Das ist Text   │
│                 │                 │
└─────────────────┴─────────────────┘
```

- Kein Umschalten zwischen Edit und Preview
- Beide Seiten synchron gescrollt
- Auf Tablet im Querformat optimal

### Rotate-Verhalten (Portrait / Landscape)

```
Landscape (quer):
  ┌─────────────┬─────────────┐
  │  EDITOR     │  PREVIEW    │
  └─────────────┴─────────────┘
  - Split-View, beide sichtbar -

Portrait (hoch) - Fokus bestimmt was angezeigt wird:
  Fokus auf Editor  → Editor fullscreen, Preview versteckt
  Fokus auf Preview → Preview fullscreen, Editor versteckt

Wechsel zwischen Editor und Preview im Portrait:
  → Swipe nach links   = Preview einblenden
  → Swipe nach rechts  = Editor einblenden
  - wie Buchseiten blättern -
```

Kein Button. Keine Einstellung. Einfach drehen und swipen.

---

### Ausblick: Forge Reader (separater Use Case)

Portrait + nur Preview = Lesemodus.
Könnte dieselbe App sein - einfach ohne Editor-Komponente gestartet.

```
forge-writer --mode editor   ← schreiben
forge-writer --mode reader   ← lesen (nur Preview, kein Editor)
```

Oder als eigene App `Forge Reader` die denselben Renderer nutzt.
Entscheidung später - Architektur erlaubt beides.

### Themes (zur Laufzeit umschaltbar)

```
Theme {
  current: "dark"    ← dark / light / sepia
}
```

| Theme | Hintergrund | Text | Wann |
|-------|------------|------|------|
| dark  | dunkel     | hell | abends, wenig Licht |
| light | weiß       | schwarz | tagsüber, Sonne |
| sepia | gelbliches Papier | dunkelbraun | angenehm neutral |

### Theme-Konsistenz über Forge-Apps

Godot nutzt RichTextLabel - andere Technologie als CrowdBooks (Web/CSS).
Direkte Kompatibilität nicht möglich, aber:

```
Forge Theme Standard:
  Gleiche Farbnamen      ← dark / light / sepia
  Gleiche Farbwerte      ← HEX-Werte definiert in forge-theme.sml
  Gleiche Semantik       ← background, text, accent, muted

forge-theme.sml:
  Theme {
    name: "dark"
    background: "#1a1a2e"
    text: "#e0e0e0"
    accent: "#7c6af7"
    muted: "#888888"
  }
  Theme {
    name: "sepia"
    background: "#f4ecd8"
    text: "#3b2a1a"
    accent: "#8b6914"
    muted: "#9a8070"
  }
```

CrowdBooks (Web) und Forge Writer (Godot) nutzen dieselben Farbwerte -
unterschiedliche Implementierung, aber gleiches visuelles Erlebnis.

User-Tracking: welche Themes werden genutzt? → Daten fließen in CrowdBooks-Design zurück.
Langfristig: Theme-Wahl des Users in beiden Apps synchron.

---

## Verzeichnisstruktur

```
~/Books/
├── OFC-Handbuch/
│   ├── de/
│   │   ├── kapitel-01.md
│   │   └── kapitel-02.md
│   └── en/
│       └── chapter-01.md
├── Brahmacharya/
│   └── de/
│       └── einleitung.md
└── 2026/
    └── en/
        └── preface.md
```

Die App zeigt den Buch-Baum als Sidebar.
Datei antippen → öffnet im Editor.

---

## Git-Anbindung (Codeberg REST API)

### Einrichtung (einmalig)

```
Settings {
  repo_url: "https://codeberg.org/art/books"
  branch: "main"
  author_name: "Art"
  author_email: "art@..."
  token: "***"    ← Codeberg Access Token
}
```

Kein git binary auf dem Gerät. Alles via REST.

---

## Sidebar - Tree View (3 Ebenen)

```
📚 Meine Bücher
├── 📖 OFC-Handbuch          ← Ebene 1: Buchtitel
│   ├── 🌐 de                ← Ebene 2: Sprache
│   │   ├── kapitel-01.md    ← Ebene 3: Kapitel
│   │   └── kapitel-02.md
│   └── 🌐 en
│       └── chapter-01.md
├── 📖 Brahmacharya
│   └── ...                  ← zugeklappt, kein Aufklappen nötig
└── 📖 2026
    └── ...
```

- Bücher die nicht interessieren → einfach zugeklappt lassen
- Beim Start: alle Bücher via REST geladen (nur Struktur, kein Inhalt)

### Buttons im Tree View

```
Ebene 1 (Buchtitel):
  [↓ Pull]      ← ganzes Buch laden (alle Kapitel)
  [↑ Push]      ← alle geänderten Kapitel pushen

Ebene 3 (Kapitel):
  [↑ Push]      ← nur dieses Kapitel pushen
                   disabled wenn keine Änderung
```

---

## Editor - Speichern & Push Logik

### Auto-Save

```
Jeder Tastendruck → sofort lokal gespeichert
- kein "Speichern"-Button nötig -
- Undo/Redo funktioniert (Standard Arrow-Buffer) -
```

### Geänderte Dateien - Statusanzeige

```
kapitel-01.md   ●  ← geändert, Push möglich
kapitel-02.md      ← unverändert, Push disabled
kapitel-03.md   ●  ← geändert, Push möglich
```

Punkt neben Dateiname = ungesyncte Änderung.

### Push-Flow

```
1. Kapitel editieren (auto-saved lokal)
2. [↑ Push] bei Kapitel oder Buch
3. Commit Message: auto "Update kapitel-01.md"
   oder eigene eingeben
4. REST API → Codeberg
5. Push-Button disabled
6. Punkt verschwindet
```

### Pull-Flow

```
1. [↓ Pull] bei Buch
2. Alle Kapitel via REST laden
3. Lokal gespeichert → auch offline editierbar
```

---

## SML Struktur (Entwurf)

```
App {
  name: "Forge Writer"
  version: "1.0"

  Layout {
    type: "split"
    ratio: 50,50

    Editor {
      syntax: "markdown"
      theme: current
      padding: 16,16,16,16
    }

    Preview {
      renderer: "markdown"
      theme: current
      padding: 16,16,16,16
      scroll_sync: true
    }
  }

  Sidebar {
    type: "filetree"
    root: "~/Books"
    filter: "*.md"
  }

  Toolbar {
    ThemeToggle { options: "dark,light,sepia" }
    GitPush { label: "Commit & Push" }
  }

  Git {
    repo: settings.repo_url
    branch: settings.branch
    auto_message: "Update {filename}"
  }
}
```

---

## Forge CLI - APK Build

### Ziel

Kein Godot GUI öffnen.
Kein manuelles Export-Template klicken.
Ein Befehl. Eine APK.

### Befehl

```bash
forge publish --target apk --project ./forge-writer
```

### Was passiert intern

```
1. Forge CLI liest project.sml
2. Godot Export Template wird aufgerufen (headless)
3. Android SDK / Gradle baut die APK
4. Output: ./dist/forge-writer.apk
```

### Voraussetzungen (einmalig einrichten)

```
- Godot Android Export Template installiert
- Android SDK vorhanden (oder via Forge CLI auto-install)
- Java/Gradle vorhanden
- forge.config {
    android_sdk: "/path/to/sdk"
    godot_template: "/path/to/template"
  }
```

### CLI Befehle (Übersicht)

```bash
forge publish --target apk     ← Android APK
forge publish --target linux   ← Linux Binary
forge publish --target win     ← Windows EXE (geplant)
forge run                      ← lokal starten
forge new MyApp                ← neues Projekt anlegen
```

---

## Offene Fragen / Entscheidungen

- [x] **Scroll-Sync:** nach Chapter - Markdown-Anker (`#`, `##`, `###`)
      Editor und Preview scrollen zum selben Heading-Anker.
      Einfach zu implementieren, stabil, kein Zeilen-Tracking nötig.

- [x] **APK:** Sideload - kein Play Store erstmal.

- [ ] **Git: lokal oder API?**
      Zwei Optionen - muss entschieden werden:

      ```
      Option A: Lokales Repo (git clone auf Tablet)
        + vollständiger Git-Workflow
        + offline arbeiten möglich
        + Konfliktbehandlung: Pull beim Start
        - braucht git binary auf Android

      Option B: Codeberg API (kein lokales Repo)
        + kein git nötig auf dem Gerät
        + simpel: Datei lesen / schreiben via REST
        - kein Offline-Modus
        - kein echter Git-History-Zugriff
      ```

      Empfehlung: Option B für v1 - simpel, kein Git-Binary-Problem.
      Option A als v2 wenn Offline-Bedarf entsteht.

- [x] **Mehrere Repos:** Ja - explizit wechseln, nicht automatisch.
      Beispiele:
      ```
      Repo A: Open Books    ← Pull Requests willkommen, Community schreibt mit
      Repo B: Closed Books  ← privat, kein Fremdzugriff (z.B. "The Third Attempt")
      Repo C: OFC Books     ← kollaborativ aber kuratiert
      ```
      User wählt aktives Repo → lädt Struktur → editiert → pusht.
      Expliziter Wechsel - kein versehentliches Pushen ins falsche Repo.

- [ ] **Bildunterstützung (v2):** Page Image pro Kapitel
      - oberstes Bild im Kapitel, wie Blog-Header
      - genutzt für SEO, OG-Tags, Social Sharing (Telegram, X etc.)
      - in CrowdBooks bereits so implementiert
      - erstmal kein Muss, aber einplanen in der Datenstruktur

---

## Meilensteine

```
M1   Split-View Editor + Preview läuft auf Android
M2   Theme-Wechsel zur Laufzeit
M3   Filetree Sidebar mit Buch-Struktur
M4   Git Commit & Push funktioniert
M5   forge publish --target apk aus CLI
```

---

*libera - Forge 4D, 2026*

---

## Soft Keyboard Layout (ohne externe Tastatur)

```
┌───────────────────────────────────┐
│  EDITOR fullscreen                │
│                                   │
├───────────────────────────────────┤
│  Markdown Toolbar                 │
├───────────────────────────────────┤
│  📱 SOFT KEYBOARD                 │
└───────────────────────────────────┘
```

Preview per Swipe erreichbar - kein Split-View wenn Soft Keyboard aktiv.

### Markdown Toolbar - Buttons

**v1 - direkt einfügen:**

| Button | Einfügt | Anmerkung |
|--------|---------|-----------|
| `#` | `# ` / `## ` / `### ` | Popup: H1 / H2 / H3 |
| `**` | `**text**` | Bold |
| `*` | `*text*` | Italic |
| ` ``` ` | ` ```\n\n``` ` | Code Block |
| `---` | `---` | Divider |
| `\|` | Basis-Tabelle | |

**v2 - mit Dialog:**

| Button | Dialog | Einfügt |
|--------|--------|---------|
| `🔗` | URL + Anzeigetext | `[Anzeigetext](https://...)` |
| `✉️` | E-Mail-Adresse + Anzeigetext | `[Anzeigetext](mailto:name@example.com)` |
| `🖼️` | URL + Alt-Text | `![Alt-Text](https://...)` |

### Dialog-Beispiel Hyperlink

```
┌─────────────────────────┐
│  Link einfügen          │
│  ─────────────────────  │
│  URL:    [____________] │
│  Text:   [____________] │
│                         │
│  [Abbrechen] [Einfügen] │
└─────────────────────────┘
```

### Dialog-Beispiel E-Mail

```
┌─────────────────────────┐
│  E-Mail einfügen        │
│  ─────────────────────  │
│  E-Mail: [____________] │
│  Text:   [____________] │
│                         │
│  [Abbrechen] [Einfügen] │
└─────────────────────────┘
→ mailto:name@example.com
```

---

## Tastatur-Erkennung - Adaptive Toolbar

Die App erkennt automatisch ob eine externe Tastatur angeschlossen ist
und platziert die Markdown-Toolbar entsprechend:

```
MIT externer Tastatur (Landscape):
  → Markdown Toolbar in der Sidebar links
  → Split-View Editor + Preview
  → kein Soft Keyboard, keine Toolbar darüber

OHNE externe Tastatur:
  → Markdown Toolbar über dem Soft Keyboard
  → Editor fullscreen
  → Sidebar zugeklappt
```

Dieselben Buttons - zwei verschiedene Orte.
Automatisch. Kein manuelles Umschalten.
