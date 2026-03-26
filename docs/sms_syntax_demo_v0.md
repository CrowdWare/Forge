# SMS Syntax Demo v0

Diese Datei zeigt die notwendigen Features als fühlbare Syntax-Snippets.

## 1) Module, Import, Export

```sms
// math.sms
export fun add(a: Int32, b: Int32): Int32 {
    return a + b
}

export var appName: String = "ForgePoser"
```

```sms
// main.sms
import "./math.sms" as math

fun ready() {
    var x: Int32 = math.add(20, 22)
    log.success("${math.appName} ready with ${x}")
}
```

## 2) Typisierung (gradual)

```sms
var count: Int32 = 0
var speed: Float32 = 1.25
var title: String = "Scene"
var isDirty: Bool = false
```

```sms
fun mul(a: Int32, b: Int32): Int32 {
    return a * b
}
```

```sms
// type error example (strict mode)
var shouldBeFast: Int23 = 5
```

## 3) Event-First (SMS-Kern)

```sms
on menuSave.clicked() {
    saveProject()
}

on menuSaveAs.clicked() {
    saveProjectAs()
}
```

Mit Event-Parametern:

```sms
on treeview.itemSelected(path: String) {
    log.info("selected: ${path}")
}
```

Super-Dispatch:

```sms
on mainWindow.sizeChanged(w: Int32, h: Int32) {
    layout.update(w, h)
    super(w, h)
}
```

## 4) Control Flow

```sms
fun sumTo(n: Int32): Int64 {
    var sum: Int64 = 0
    for (var i: Int32 = 0; i < n; i = i + 1) {
        sum = sum + i
    }
    return sum
}
```

```sms
var list = [ "tree", "rock", "chair" ]
for (item in list) {
    log.info(item)
}
```

```sms
fun modeLabel(mode: String): String {
    when (mode) {
        "pose" -> return "Pose Mode"
        "move" -> return "Move Mode"
        "anim" -> return "Animation Mode"
        else -> return "Unknown Mode"
    }
}
```

## 5) Named Args (Kotlin-familiar, optional)

```sms
ui.openFileDialog(
    title: "Open Scene",
    filter: "*.scene",
    onSelected: "loadScene"
)
```

Mischform (gültig):

```sms
var vec = Vec3D(10, y = 100)   // positional first, named afterwards
```

Nur named (gültig):

```sms
var vec = Vec3D(y = 100)        // x/z from defaults, y overridden
```

Ungültig:

```sms
var vec = Vec3D(y = 100, 10)    // positional after named
var vec = Vec3D(y = 100, y = 5) // duplicate named arg
var vec = Vec3D(speed = 10)     // unknown named arg
```

Empfohlene Fehlermeldungen:

- `Positional argument is not allowed after named arguments.`
- `Duplicate named argument 'y'.`
- `Unknown named argument 'speed' for 'Vec3D'.`

## 6) Deterministic Friendly Subset (Game-Profil)

```sms
// no hidden async, explicit update tick
fun update(deltaMs: Int32) {
    playerX = playerX + velocityX
}
```

## 7) Tuple Return (Error Handling ohne try-catch)

Funktionen können ein Tuple zurückgeben – immer `(Bool, String, ...)` als Konvention:

```sms
fun fetchData(url: String): (Bool, String, Data) {
    // bei Fehler:
    return (false, "Netzwerk nicht erreichbar", null)
    // bei Erfolg:
    return (true, "ok", result)
}
```

Aufrufer destrukturiert direkt:

```sms
var (ok, msg, data) = fetchData("https://example.com")
if (!ok) {
    log.error(msg)
}
```

Kurzform für einfache Fälle (nur Bool + Nachricht):

```sms
fun saveFile(path: String): (Bool, String) {
    return (true, "saved")
}

var (ok, msg) = saveFile("/tmp/scene.forge")
```

Kein try-catch, kein null-Crash, deterministisch. Der Aufrufer entscheidet, was mit dem Fehler passiert.

## 8) Relaxed vs Strict Mode (Compiler/Transpiler)

Relaxed:
- unknown type -> warning
- implicit number widening -> warning

Strict:
- unknown type -> error
- unsafe implicit conversion -> error
