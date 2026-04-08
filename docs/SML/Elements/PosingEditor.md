# PosingEditor

## Inheritance

[PosingEditor](PosingEditor.md) → [SubViewportContainer](SubViewportContainer.md) → [Container](Container.md) → [Control](Control.md) → [CanvasItem](CanvasItem.md) → [Node](Node.md) → [Object](Object.md)

## Properties

This page lists **only properties declared by `PosingEditor`**.
Inherited properties are documented in: [SubViewportContainer](SubViewportContainer.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|
| - | id | identifier | - |
| - | src | string(url) | "" |
| - | showBoneTree | bool | false |
| - | normalizeNames | bool | true |

## Events

This page lists **only signals declared by `PosingEditor`**.
Inherited signals are documented in: [SubViewportContainer](SubViewportContainer.md)

| Godot Signal | SMS Event | Params |
|-|-|-|

## Actions

This page lists **only actions supported by the runtime** for `PosingEditor`.
Inherited actions are documented in: [SubViewportContainer](SubViewportContainer.md)

| Action | SMS Call | Params | Returns |
|-|-|-|-|
| resetPose | `<id>.resetPose()` | - | void |
| setJointSpheresVisible | `<id>.setJointSpheresVisible(visible)` | bool visible | void |
| setBoneTree | `<id>.setBoneTree(treeId)` | string treeId | void |
| setModelSource | `<id>.setModelSource(src)` | string src | void |
| loadProject | `<id>.loadProject(path)` | string path | void |
| getProjectText | `<id>.getProjectText(path)` | string path | string |
| applyProjectText | `<id>.applyProjectText(path, content)` | string path, string content | bool |
| saveProject | `<id>.saveProject(path)` | string path | void |
| setProjectProperty | `<id>.setProjectProperty(key, value)` | string key, string value | void |
| getProjectProperty | `<id>.getProjectProperty(key, fallback)` | string key, string fallback | string |
| addSceneAsset | `<id>.addSceneAsset(path, posX, posY, posZ)` | string path, float posX, float posY, float posZ | int |
| addGreyboxItem | `<id>.addGreyboxItem(kind, posX, posY, posZ)` | string kind, float posX, float posY, float posZ | int |
| addSceneProp | `<id>.addSceneProp(path, posX, posY, posZ)` | string path, float posX, float posY, float posZ | int |
| getSceneCharacterCount | `<id>.getSceneCharacterCount()` | - | int |
| getActiveCharacterId | `<id>.getActiveCharacterId()` | - | string |
| getSceneCharacterPath | `<id>.getSceneCharacterPath(index)` | int index | string |
| getSceneCharacterName | `<id>.getSceneCharacterName(index)` | int index | string |
| getSceneCharacterVisible | `<id>.getSceneCharacterVisible(index)` | int index | bool |
| selectSceneCharacter | `<id>.selectSceneCharacter(index)` | int index | void |
| removeCharacter | `<id>.removeCharacter()` | - | void |
| removeSceneCharacter | `<id>.removeSceneCharacter(index)` | int index | void |
| removeSceneProp | `<id>.removeSceneProp(index)` | int index | void |
| getScenePropCount | `<id>.getScenePropCount()` | - | int |
| getScenePropPath | `<id>.getScenePropPath(index)` | int index | string |
| getScenePropName | `<id>.getScenePropName(index)` | int index | string |
| getScenePropVisible | `<id>.getScenePropVisible(index)` | int index | bool |
| getScenePropPos | `<id>.getScenePropPos(index)` | int index | string |
| setScenePropPos | `<id>.setScenePropPos(index, x, y, z)` | int index, float x, float y, float z | void |
| setSceneCharacterVisible | `<id>.setSceneCharacterVisible(index, visible)` | int index, bool visible | void |
| setScenePropVisible | `<id>.setScenePropVisible(index, visible)` | int index, bool visible | void |
| getScenePropRot | `<id>.getScenePropRot(index)` | int index | string |
| setScenePropRot | `<id>.setScenePropRot(index, x, y, z)` | int index, float x, float y, float z | void |
| placeSelectedOnGround | `<id>.placeSelectedOnGround(groundY)` | float groundY | bool |
| rebaseSelectedPivotBottom | `<id>.rebaseSelectedPivotBottom()` | - | bool |
| getPoseDataForCharacter | `<id>.getPoseDataForCharacter(characterId)` | string characterId | object |
| getPoseDataForActiveCharacter | `<id>.getPoseDataForActiveCharacter()` | - | object |
| getPoseDataForBone | `<id>.getPoseDataForBone(boneName)` | string boneName | object |
| getPoseDataForSelectedBone | `<id>.getPoseDataForSelectedBone()` | - | object |
| setMode | `<id>.setMode(mode)` | string mode | void |
| setEditMode | `<id>.setEditMode(mode)` | string mode | void |
| exportGlb | `<id>.exportGlb(path, includeAnimation, includeProps, animationOnlyCharacter)` | string path, bool includeAnimation, bool includeProps, bool animationOnlyCharacter | bool |
| exportCurrentFramePng | `<id>.exportCurrentFramePng(path)` | string path | bool |
| exportFrameRangePng | `<id>.exportFrameRangePng(frameFrom, frameTo, outputDirectory)` | int frameFrom, int frameTo, string outputDirectory | int |
| startExportFrameRangePng | `<id>.startExportFrameRangePng(frameFrom, frameTo, outputDirectory)` | int frameFrom, int frameTo, string outputDirectory | bool |
