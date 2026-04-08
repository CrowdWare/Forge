# VideoStreamPlayer

## Inheritance

[VideoStreamPlayer](VideoStreamPlayer.md) → [Control](Control.md) → [CanvasItem](CanvasItem.md) → [Node](Node.md) → [Object](Object.md)

## Properties

This page lists **only properties declared by `VideoStreamPlayer`**.
Inherited properties are documented in: [Control](Control.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|
| audio_track | audioTrack | int | - |
| autoplay | autoplay | bool | - |
| buffering_msec | bufferingMsec | int | - |
| expand | expand | bool | - |
| loop | loop | bool | - |
| paused | paused | bool | - |
| speed_scale | speedScale | float | - |
| volume_db | volumeDb | float | - |

## Events

This page lists **only signals declared by `VideoStreamPlayer`**.
Inherited signals are documented in: [Control](Control.md)

| Godot Signal | SMS Event | Params |
|-|-|-|
| finished | `on <id>.finished() { ... }` | - |

## Runtime Actions

This page lists **callable methods declared by `VideoStreamPlayer`**.
Inherited actions are documented in: [Control](Control.md)

| Godot Method | SMS Call | Params | Returns |
|-|-|-|-|
| get_stream_length | `<id>.getStreamLength()` | - | float |
| get_stream_name | `<id>.getStreamName()` | - | string |
| get_video_texture | `<id>.getVideoTexture()` | - | Object |
| has_autoplay | `<id>.hasAutoplay()` | - | bool |
| has_expand | `<id>.hasExpand()` | - | bool |
| has_loop | `<id>.hasLoop()` | - | bool |
| is_playing | `<id>.isPlaying()` | - | bool |
| play | `<id>.play()` | - | void |
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

