# ForgeTimelineControl

## Inheritance

[ForgeTimelineControl](ForgeTimelineControl.md) → [SubViewportContainer](SubViewportContainer.md) → [Container](Container.md) → [Control](Control.md) → [CanvasItem](CanvasItem.md) → [Node](Node.md) → [Object](Object.md)

## Properties

This page lists **only properties declared by `ForgeTimelineControl`**.
Inherited properties are documented in: [SubViewportContainer](SubViewportContainer.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|
| currentFrame | currentFrame | int | - |
| fps | fps | int | - |
| totalFrames | totalFrames | int | - |

## Events

This page lists **only signals declared by `ForgeTimelineControl`**.
Inherited signals are documented in: [SubViewportContainer](SubViewportContainer.md)

| Godot Signal | SMS Event | Params |
|-|-|-|
| frameChanged | `on <id>.frameChanged(frame) { ... }` | int frame |
| keyframeAdded | `on <id>.keyframeAdded(frame, boneName) { ... }` | int frame, string boneName |
| keyframeRemoved | `on <id>.keyframeRemoved(frame) { ... }` | int frame |
| playbackStarted | `on <id>.playbackStarted() { ... }` | - |
| playbackStopped | `on <id>.playbackStopped() { ... }` | - |

## Runtime Actions

This page lists **callable methods declared by `ForgeTimelineControl`**.
Inherited actions are documented in: [SubViewportContainer](SubViewportContainer.md)

| Godot Method | SMS Call | Params | Returns |
|-|-|-|-|
| clearAllKeyframes | `<id>.clearAllKeyframes()` | - | void |
| debugLogKeyframesForCharacter | `<id>.debugLogKeyframesForCharacter(characterId)` | string characterId | void |
| getKeyframeBoneCountForCharacter | `<id>.getKeyframeBoneCountForCharacter(frame, characterId)` | int frame, string characterId | int |
| getKeyframeCount | `<id>.getKeyframeCount()` | - | int |
| getKeyframeCountForCharacter | `<id>.getKeyframeCountForCharacter(characterId)` | string characterId | int |
| getKeyframeFrameAt | `<id>.getKeyframeFrameAt(index)` | int index | int |
| getKeyframeFrameAtForCharacter | `<id>.getKeyframeFrameAtForCharacter(index, characterId)` | int index, string characterId | int |
| getPoseAt | `<id>.getPoseAt(frame)` | int frame | void |
| get_current_frame_prop | `<id>.getCurrentFrameProp()` | - | int |
| get_total_frames | `<id>.getTotalFrames()` | - | int |
| hasKeyframeAt | `<id>.hasKeyframeAt(frame)` | int frame | bool |
| isPlaying | `<id>.isPlaying()` | - | bool |
| play | `<id>.play()` | - | void |
| removeKeyframe | `<id>.removeKeyframe(frame)` | int frame | void |
| setCurrentFrame | `<id>.setCurrentFrame(frame)` | int frame | void |
| setKeyframeMove | `<id>.setKeyframeMove(frame)` | int frame | void |
| setKeyframeRotate | `<id>.setKeyframeRotate(frame)` | int frame | void |
| setVisibleCharacterId | `<id>.setVisibleCharacterId(characterId)` | string characterId | void |
| set_current_frame_prop | `<id>.setCurrentFrameProp(value)` | int value | void |
| set_total_frames | `<id>.setTotalFrames(value)` | int value | void |
| stop | `<id>.stop()` | - | void |

## Attached Properties

These properties are declared by a parent provider and set on this element using the qualified syntax `<providerId>.property: value` or `ProviderType.property: value`.

### Provided by `TabContainer`

| Attached Property | Type | Description |
|-|-|-|
| title | string | Tab title read by the parent TabContainer. Use attached property syntax: `<containerId>.title: "Caption"` or `TabContainer.title: "Caption"`. |

### Provided by `DockingContainer`

| Attached Property | Type | Description |
|-|-|-|
| title | string | Tab title read by the parent DockingContainer. Use attached property syntax: `<containerId>.title: "Caption"` or `DockingContainer.title: "Caption"`. |

