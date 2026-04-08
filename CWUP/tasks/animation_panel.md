# Forge: PosingEditor & Timeline Controls - Claude Code Task

## Context

Forge is a UI framework that parses SML (Simple Markup Language, similar to QML) and instantiates
Godot objects. It already has:

- A working `Viewport3D` control that loads `.glb` models and plays `.glb` animations
- SML markup parsed into Godot nodes at runtime
- SMS (Simple Multiplatform Script, Kotlin-based interpreter in a sandbox) for event logic

The goal is to build two new Forge controls: **PosingEditor** and **Timeline**, plus a shared
**RotationGizmo3D** node that both controls depend on.

---

## 1. RotationGizmo3D

A runtime 3D rotation gizmo rendered directly in the Godot viewport. Inspired by Blockbench's gizmo.

### Visual Design
- Three torus rings, one per axis:
  - X axis → red (`TorusMesh`)
  - Y axis → green (`TorusMesh`)
  - Z axis → blue (`TorusMesh`)
- Each ring has a small draggable **handle** (`SphereMesh`) positioned at 90° on the ring
- Only the handle is interactive - dragging it rotates the bone on that axis
- The gizmo renders on top of the model (no depth occlusion)

### Behavior
- Attach to a `Skeleton3D` bone by index
- Position itself at the bone's global transform origin every frame
- On handle drag: rotate the bone on the corresponding axis
- **Min/Max limits per axis** - the ring visually shows the allowed arc, rotation is clamped:
  ```
  gizmo.limitX(minDeg: Float, maxDeg: Float)
  gizmo.limitY(minDeg: Float, maxDeg: Float)
  gizmo.limitZ(minDeg: Float, maxDeg: Float)
  ```
- When a limit is reached, the handle stops and the ring turns orange as visual feedback
- Gizmo is hidden when no bone is selected

### Godot References to study
- `editor/plugins/skeleton_3d_editor_plugin.cpp` - Godot's own bone gizmo implementation
- `scene/3d/skeleton_3d.cpp` - `set_bone_pose_rotation()`, `get_bone_global_pose()`
- `SkeletonIK3D` - for automatic IK solving after manual bone rotation

---

## 2. PosingEditor Control

A Forge control that combines a 3D viewport, a bone tree panel, and the RotationGizmo3D into one
self-contained posing tool.

### SML Usage
```
PosingEditor {
    src: "res://assets/models/demo.glb"
    showBoneTree: true
}
```

### Sub-components

#### 2a. 3D Viewport (left/center area)
- Renders the loaded `.glb` model with its `Skeleton3D`
- Displays **pickable joint spheres** at every bone position (`SphereMesh`, semi-transparent)
- On joint click (raycasting via `PhysicsDirectSpaceState3D`):
  - Selects the bone
  - Shows the `RotationGizmo3D` at that joint
  - Highlights the corresponding entry in the BoneTree
- Camera: orbit with mouse drag, zoom with scroll wheel, touch-friendly for tablet

#### 2b. BoneTree Panel (right side)
- Hierarchical tree of all bones read from `Skeleton3D.get_bone_name()` / `get_bone_parent()`
- Click on a bone entry → selects it, shows gizmo in viewport
- Selected bone is highlighted
- Collapsible bone groups

#### 2c. Pose Data Model
- Internal dictionary: `boneName → Quaternion`
- Updated on every gizmo interaction
- Readable via SML property `editor.poseData`

### Joint Constraint System
Constraints are defined in SML and applied to the gizmo:
```
PosingEditor {
    src: "res://assets/models/demo.glb"

    JointConstraint { bone: "RightKnee";  minX: -140; maxX: 0; minY: -30; maxY: 30 }
    JointConstraint { bone: "LeftKnee";   minX: -140; maxX: 0; minY: -30; maxY: 30 }
    JointConstraint { bone: "RightElbow"; minX: -145; maxX: 0; minY: 0;   maxY: 0  }
    JointConstraint { bone: "LeftElbow";  minX: -145; maxX: 0; minY: 0;   maxY: 0  }
}
```

### IK Support
- Use Godot's `SkeletonIK3D` as a child of `Skeleton3D`
- When a hand or foot bone is moved, IK automatically resolves the chain
- IK chain length configurable per limb

### Events (SMS syntax)
```kotlin
on editor.boneSelected(boneName) { }
on editor.poseChanged(boneName, rotation) { }
on editor.poseReset() { }
```

### Methods
```kotlin
editor.resetPose()
editor.exportPoseAsGLB(path)
editor.loadPose(poseData)        // load a previously saved SML pose
editor.savePoseAsSML()           // returns SML string of current pose
```

### Pose saved as SML (for AI context feeding)
```
Pose {
    source: "res://assets/models/demo.glb"
    Bone { name: "Hips";        rotX: 0;    rotY: 15;   rotZ: 0   }
    Bone { name: "Spine";       rotX: 10;   rotY: 0;    rotZ: 0   }
    Bone { name: "LeftArm";     rotX: -45;  rotY: 0;    rotZ: 90  }
    Bone { name: "RightArm";    rotX: -45;  rotY: 0;    rotZ: -90 }
    Bone { name: "RightKnee";   rotX: -90;  rotY: 0;    rotZ: 0   }
}
```

---

## 3. Timeline Control

A Forge control for keyframe-based animation editing.

### SML Usage
```
Timeline {
    fps: 24
    totalFrames: 120
}
```

### Visual Layout
- Horizontal scrollable track area
- One track per bone that has keyframes
- Keyframe diamonds on the track at their frame position
- Playhead (vertical line) draggable to scrub
- Play / Pause / Stop buttons
- Frame counter display

### Behavior
- **Set Keyframe**: captures current `PosingEditor.poseData` at current frame
- **Scrub**: moving playhead interpolates between keyframes and applies pose to model
- **Interpolation**: Linear by default, with Ease In/Out option per keyframe
- **Delete Keyframe**: select diamond + delete key
- Multiple bones can share the same frame or have independent keyframe positions

### Events (SMS syntax)
```kotlin
on timeline.frameChanged(frame) { }
on timeline.keyframeAdded(frame, boneName) { }
on timeline.keyframeRemoved(frame, boneName) { }
on timeline.playbackStarted() { }
on timeline.playbackStopped() { }
```

### Methods
```kotlin
timeline.setKeyframe(frame, poseData)
timeline.removeKeyframe(frame)
timeline.play()
timeline.stop()
timeline.exportAnimationAsGLB(path)
```

---

## 4. Scene Builder (optional, Phase 2)

Allow dragging additional `.glb` objects into the PosingEditor scene to build a stage/environment.

### SML Usage
```
PosingEditor {
    src: "res://assets/models/character.glb"
    SceneAssets {
        assetsPath: "res://assets/props/"
    }
}
```

- Asset panel showing thumbnails of available `.glb` props
- Drag & drop into viewport to place object
- Selected objects get a standard move/scale gizmo (not rotation gizmo)
- Scene exported as single `.glb` including character pose + props

---

## 5. Full SML Example

```
Window {
    Column {
        PosingEditor {
            id: "editor"
            src: "res://assets/models/demo.glb"
            showBoneTree: true

            JointConstraint { bone: "RightKnee"; minX: -140; maxX: 0 }
            JointConstraint { bone: "LeftKnee";  minX: -140; maxX: 0 }
        }

        Timeline {
            id: "timeline"
            fps: 24
            totalFrames: 120
        }
    }
}
```

### SMS wiring
```kotlin
on editor.poseChanged(boneName, rotation) {
    // auto-set keyframe on pose change
    timeline.setKeyframe(timeline.currentFrame, editor.poseData)
}

on timeline.frameChanged(frame) {
    editor.loadPose(timeline.getPoseAt(frame))
}
```

---

## 6. Godot Source References

Study these before implementing:

| File | Relevant for |
|---|---|
| `editor/plugins/skeleton_3d_editor_plugin.cpp` | Bone gizmo, picking, SubGizmo pattern |
| `scene/3d/skeleton_3d.cpp` | `set_bone_pose_rotation()`, bone transforms |
| `scene/3d/skeleton_ik_3d.cpp` | IK solver integration |
| `modules/gltf/gltf_document.cpp` | Runtime GLB export with skeleton pose |
| `github.com/thimenesup/BoneGizmo` | Simple reference implementation in Godot |

---

## 7. What Godot Does NOT Have (must be built)

| Missing piece | Notes |
|---|---|
| Runtime RotationGizmo3D with min/max limits | Godot's gizmo only works in editor, not runtime |
| Pickable joint spheres at runtime | Must be placed manually on bone positions each frame |
| SML-driven JointConstraint system | Custom Forge node |
| Timeline control | No Godot runtime equivalent of AnimationTrackEditor |
| Pose → SML serialization | Custom serializer |
| AI-readable pose format | SML Pose block (see section 2) |

---

## 8. Platforms

Must work on:
- Desktop (Windows, macOS, Linux)
- Web (Godot HTML5 export)
- Tablet (Android / iPad via Godot mobile export)

Touch input must work for: bone picking, gizmo handle dragging, timeline scrubbing.


## 9. TestEnvironment
We will create a new Forge-app in the folder /ForgePoser.
There we will use these new controls in an animation creation tools.
The export output will be a .glb with an animation.
And maybe a scene (phase 2) as .glb.

---

## 10. Requirements & completetd and open 

### Completed
- RotationGizmo3D (Torus-Ringe, Handles, World-Space-Rotation, Limits, Orange-Feedback, Winkel-Label)
- PosingEditorControl (Orbit-Kamera, Joint-Spheres, Picking, BoneTree als collapsible Tree, Constraints)
- TimelineControl (Ruler, Bone-Tracks, Keyframe-Diamonds, Playhead, Play/Stop, lineare Interpolation)
- SMS-Verdrahtung: `poseChanged → setKeyframe`, `frameChanged → loadPose`
- ForgePoser App: `main.sml` + `main.sms`

### Completed (Phase 2)

| Feature | Aufwand | Hinweis |
|---|---|---|
| `btnOpen` - Datei-Dialog um GLB zu laden | mittel | `ui.openFileDialog(callback)` in SmsUiRuntime; `main.sms` Handler |
| Keyframe löschen (UI-Geste) | klein | Rechtsklick auf Diamond-Position oder Del-Taste in TimelineTrackArea |
| SMS-Events `keyframeAdded` / `keyframeRemoved` | klein | In `SetKeyframe`/`RemoveKeyframe` gefeuert; Dispatcher + SmsUiRuntime-Handler |


### Completed (Phase 3)

| Feature | Aufwand | Hinweis |
|---|---|---|
| Instead of save as *.fpose save all as a *.scene | mittel | This allows us to use *.scene instead of *.tscn (see sample below)|
| Move Gizmo | gross | Should look like in Blockbench, 3 colors, 3 directions |
| Place objects in scene| mittel | all objects in scene should be movable and rotatable |

# Bugfixes for Phase 3
- Timeline Control renders outside its box.
- Scene does not support float vectors for pos, rot and scale
```sml
pos: 2.0, 2.0, 0.0 
rot: 0.0, 0.0, 0.0 
scale: 1.0, 1.0, 1.0
```
- Assets are positioned at 0,0,0 regardless of their pos in the scene. This might also be true for rotation and scale, because these values are defined as integer atm. 
- When I select an asset in the scene list, it is not selected and no gizmo is displayed. Fix: draw a border around the asset in the viewport and display the desired gizmo, based on the mode (move, rotatet or scale).
  
  
### Still open (Phase 4)
| Feature | Aufwand | Hinweis |
|---|---|---|
| Export Dialog Centered|small| Display the dialog centered on the screen and also make it a big bigger - Take Save As Dialog as reference |
| Change Export Dialog| small | User shall be able to choose between, the whole scene incl. animation, only the character and with or without animation - He might also choose character animation only for retargeting - Greybox assets might only be useful to as preview only depends on the use case |
| `exportPoseAsGLB(path)` | huge | Godot `GltfDocument` mit gesetzten Bone-Poses exportieren, still not good enough, because loading it in blender show Armature as a golf ball like mesh which hides the real mesh. See (### Last words from Claude) below|
| `exportAnimationAsGLB(path)` | huge | Keyframes → `AnimationLibrary` → `GltfDocument` exportieren |
| Import animation / retarget animation | mittel | Load an animation from a .glb and use it on a character, animation will then be used and saved in the scene |
| IK-Support (`SkeletonIK3D`) | huge | Godot `SkeletonIK3D` als Kind von Skeleton3D; Hand/Fuß-Knochen triggern IK-Solve |


### Last words from Claude
Ah danke - das ändert die Diagnose komplett.

Die Scale 10 kommt also von Dir in Blender, um den Mesh sichtbar zu machen. Das bedeutet der exportierte Mesh ist ~10× zu klein verglichen mit dem Skeleton. Das ist eine Bind-Matrix-Mismatch: in Godot's GLTF-Roundtrip (AppendFromScene) werden die Bone-Positionen und die Mesh-Vertex-Positionen in unterschiedlichen Koordinatenräumen exportiert.

Dazu sehe ich im Outliner zwei weitere Probleme:

_PhysicalBoneSimulator3D wird mitexportiert (Godot-interner Node, gehört nicht ins GLB)
Animation / Pose - das sind AnimationPlayer-Nodes aus dem original Opa.glb, die auch unnötig mitgehen
Bevor ich weiter daran arbeite: Woher stammt Opa.glb? Mixamo, ReadyPlayerMaker, selbst in Blender erstellt? Und was ist die scale im .scene-File - du hast scale: 1.0, 1.0, 1.0, aber hat der importierte Node in Godot vielleicht intern eine andere Scale? Das würde helfen, den genauen Skalierungsfaktor zu verstehen.

Den _PhysicalBoneSimulator3D und original-AnimationPlayer kann ich sofort rausfiltern - das ist eindeutig falsch. Soll ich das schon machen?

---

User: Opa ist von Mixamo.
Übrigens sehen die Objekte von Meshy genau so aus mit dem Golfball

---

Jetzt wird's klarer. Der Golfball kommt vermutlich von _PhysicalBoneSimulator3D - Mixamo-Charaktere haben Physics-Bones, und Godot erstellt dafür Sphere-Collision-Shapes. Diese werden beim Export mitgenommen und erscheinen in Blender als überlappende Kugeln → Golfball-Blob. Das erklärt auch warum Meshy-Objekte ähnlich aussehen könnten, wenn deren GLBs ebenfalls Physics-Nodes triggern.

Fix: Vor dem Export alle Physics-Nodes und Original-AnimationPlayers (von der GLB-Importierung) aus dem Export-Tree entfernen. Soll ich das implementieren?




### Scene sample
Instead of Asset and Character we might use the Godot type instead.
Node3D or whatever that is. Or PackedScene makes also sense, so that we later can include subscenes.

```sms
Scene {
    id: scene

    WorldEnvironment {
        backgroundMode = 2
        tonemapMode = 2
        glowEnabled = true

        Sky {
            ProceduralSkyMaterial {
                skyHorizonColor: "#0000FF"
                groundHorizonColor: "#BBBBFF"
            }
        }
    }

    DirectionalLight3D {
        pos: 0.0, 0.0, 0.0
        rot: 0.0, 0.0, 0.0
        scale: 1.0, 1.0, 1.0
        mode: euler // default
        shadowEnabled = true
    }

    Camera3D {
        id: camera
    }

    Asset {
        id: wall
        name: "hero"
        src: "res://assets/models/wall.glb"
        pos: 2.0, 2.0, 0.0 
        rot: 0.0, 0.0, 0.0 
        scale: 1.0, 1.0, 1.0
    }

    Character {
        id: hero
        name: "hero"
        src: "res://assets/models/Opa.glb"
        pos: 0.0, 0.0, 0.0 
        rot: 0.0, 0.0, 0.0 
        scale: 1.0, 1.0, 1.0
       
        Animation {
            target: hero
            Key { frame: 0
                Bone { name: "Head" x: 0.187317 y: -0.0257134 z: -0.00490508 w: 0.981951 }
                Bone { name: "LeftForeArm" x: 0.203757 y: -0.286319 z: 0.554261 w: 0.754519 }
                Bone { name: "LeftArm" x: 0.263918 y: 0.000433639 z: -0.016206 w: 0.964409 }
                Bone { name: "RightArm" x: 0.444904 y: 0.000967647 z: 0.0137014 w: 0.895473 }
            }
            Key { frame: 10
                Bone { name: "LeftForeArm" x: 0.292009 y: -0.405733 z: 0.865405 w: -0.0344294 }
                Bone { name: "Head" x: 0.328754 y: -0.024721 z: -0.00860875 w: 0.944053 }
                Bone { name: "LeftArm" x: 0.263918 y: 0.000433639 z: -0.016206 w: 0.964409 }
                Bone { name: "RightArm" x: 0.444904 y: 0.000967647 z: 0.0137014 w: 0.895473 }
            }
        }
    }
}
```