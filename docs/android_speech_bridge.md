# Android Speech Bridge (ForgeSpeechBridge)

This document defines the fixed host bridge contract for native Android speech recognition in Forge.

## Security Model

- No generic Java call API is exposed to SMS.
- Method names and singleton lookup are hardcoded in C++ (`ForgeSpeechBridge`).
- User scripts cannot execute arbitrary Java methods.

## Singleton Name

- `ForgeSpeechBridge`

Registered as an Engine singleton in the Android host/plugin layer.

## Required Host Methods

- `startListening(objectId: int, language: String) -> bool`
- `stopListening(objectId: int) -> void`

`objectId` is the Godot object instance id of `ForgeSpeechRecognizerControl`.

## Required Callback Methods on Control

The host must call these methods on the target object:

- `_bridgeRawResult(text: String)` for partial/intermediate text
- `_bridgeResult(text: String)` for final result
- `_bridgeError(message: String)` on failure

## Control Behavior

- `_bridgeResult` runs post-processing (`raw|clean|markdown`) inside C++ before emitting `result(text)`.
- `listen()` falls back to keyboard-mic if no `ForgeSpeechBridge` singleton is present.
