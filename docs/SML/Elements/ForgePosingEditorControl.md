# ForgePosingEditorControl

## Inheritance

[ForgePosingEditorControl](ForgePosingEditorControl.md) → [SubViewportContainer](SubViewportContainer.md) → [Container](Container.md) → [Control](Control.md) → [CanvasItem](CanvasItem.md) → [Node](Node.md) → [Object](Object.md)

## Properties

This page lists **only properties declared by `ForgePosingEditorControl`**.
Inherited properties are documented in: [SubViewportContainer](SubViewportContainer.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|
| showBoneTree | showBoneTree | bool | - |
| src | src | string | - |

## Events

This page lists **only signals declared by `ForgePosingEditorControl`**.
Inherited signals are documented in: [SubViewportContainer](SubViewportContainer.md)

| Godot Signal | SMS Event | Params |
|-|-|-|
| boneSelected | `on <id>.boneSelected(boneName) { ... }` | string boneName |
| exportProgress | `on <id>.exportProgress(filename, percent) { ... }` | string filename, int percent |
| frameRangeExportFinished | `on <id>.frameRangeExportFinished(written, outputDirectory) { ... }` | int written, string outputDirectory |
| objectMoved | `on <id>.objectMoved(propIdx, pos) { ... }` | int propIdx, string pos |
| objectSelected | `on <id>.objectSelected(propIdx) { ... }` | int propIdx |
| poseChanged | `on <id>.poseChanged(boneName) { ... }` | string boneName |
| poseReset | `on <id>.poseReset() { ... }` | - |
| scenePropAdded | `on <id>.scenePropAdded(index, path) { ... }` | int index, string path |
| scenePropRemoved | `on <id>.scenePropRemoved(index) { ... }` | int index |

## Runtime Actions

This page lists **callable methods declared by `ForgePosingEditorControl`**.
Inherited actions are documented in: [SubViewportContainer](SubViewportContainer.md)

| Godot Method | SMS Call | Params | Returns |
|-|-|-|-|
| addGreyboxItem | `<id>.addGreyboxItem(kind, x, y, z)` | string kind, float x, float y, float z | int |
| addSceneAsset | `<id>.addSceneAsset(path, x, y, z)` | string path, float x, float y, float z | int |
| applyProjectText | `<id>.applyProjectText(path, text, sync)` | string path, string text, bool sync | bool |
| exportCurrentFramePng | `<id>.exportCurrentFramePng(path)` | string path | bool |
| exportFrameRangePng | `<id>.exportFrameRangePng(frameFrom, frameTo, outputDirectory)` | int frameFrom, int frameTo, string outputDirectory | int |
| getActiveCharacterId | `<id>.getActiveCharacterId()` | - | string |
| getPoseDataForActiveCharacter | `<id>.getPoseDataForActiveCharacter()` | - | Variant |
| getPoseDataForBone | `<id>.getPoseDataForBone(boneName)` | string boneName | Variant |
| getPoseDataForSelectedBone | `<id>.getPoseDataForSelectedBone()` | - | Variant |
| getProjectText | `<id>.getProjectText(path)` | string path | string |
| getSceneCharacterCount | `<id>.getSceneCharacterCount()` | - | int |
| getSceneCharacterId | `<id>.getSceneCharacterId(index)` | int index | string |
| getSceneCharacterName | `<id>.getSceneCharacterName(index)` | int index | string |
| getSceneCharacterPosX | `<id>.getSceneCharacterPosX(index)` | int index | float |
| getSceneCharacterPosY | `<id>.getSceneCharacterPosY(index)` | int index | float |
| getSceneCharacterPosZ | `<id>.getSceneCharacterPosZ(index)` | int index | float |
| getSceneCharacterRotX | `<id>.getSceneCharacterRotX(index)` | int index | float |
| getSceneCharacterRotY | `<id>.getSceneCharacterRotY(index)` | int index | float |
| getSceneCharacterRotZ | `<id>.getSceneCharacterRotZ(index)` | int index | float |
| getSceneCharacterScaleX | `<id>.getSceneCharacterScaleX(index)` | int index | float |
| getSceneCharacterScaleY | `<id>.getSceneCharacterScaleY(index)` | int index | float |
| getSceneCharacterScaleZ | `<id>.getSceneCharacterScaleZ(index)` | int index | float |
| getScenePropCount | `<id>.getScenePropCount()` | - | int |
| getScenePropName | `<id>.getScenePropName(index)` | int index | string |
| getScenePropPosX | `<id>.getScenePropPosX(index)` | int index | float |
| getScenePropPosY | `<id>.getScenePropPosY(index)` | int index | float |
| getScenePropPosZ | `<id>.getScenePropPosZ(index)` | int index | float |
| getScenePropRotX | `<id>.getScenePropRotX(index)` | int index | float |
| getScenePropRotY | `<id>.getScenePropRotY(index)` | int index | float |
| getScenePropRotZ | `<id>.getScenePropRotZ(index)` | int index | float |
| getScenePropScaleX | `<id>.getScenePropScaleX(index)` | int index | float |
| getScenePropScaleY | `<id>.getScenePropScaleY(index)` | int index | float |
| getScenePropScaleZ | `<id>.getScenePropScaleZ(index)` | int index | float |
| getSelectedBoneMaxX | `<id>.getSelectedBoneMaxX()` | - | float |
| getSelectedBoneMaxY | `<id>.getSelectedBoneMaxY()` | - | float |
| getSelectedBoneMaxZ | `<id>.getSelectedBoneMaxZ()` | - | float |
| getSelectedBoneMinX | `<id>.getSelectedBoneMinX()` | - | float |
| getSelectedBoneMinY | `<id>.getSelectedBoneMinY()` | - | float |
| getSelectedBoneMinZ | `<id>.getSelectedBoneMinZ()` | - | float |
| getSelectedBoneName | `<id>.getSelectedBoneName()` | - | string |
| getSelectedBoneRotX | `<id>.getSelectedBoneRotX()` | - | float |
| getSelectedBoneRotY | `<id>.getSelectedBoneRotY()` | - | float |
| getSelectedBoneRotZ | `<id>.getSelectedBoneRotZ()` | - | float |
| get_show_bone_tree | `<id>.getShowBoneTree()` | - | bool |
| loadProject | `<id>.loadProject(path)` | string path | bool |
| placeSelectedOnGround | `<id>.placeSelectedOnGround(groundY)` | float groundY | bool |
| removeSceneCharacter | `<id>.removeSceneCharacter(index)` | int index | void |
| removeSceneProp | `<id>.removeSceneProp(index)` | int index | void |
| resetPose | `<id>.resetPose()` | - | void |
| saveProject | `<id>.saveProject(path)` | string path | bool |
| selectSceneCharacter | `<id>.selectSceneCharacter(index)` | int index | void |
| selectSceneProp | `<id>.selectSceneProp(index)` | int index | void |
| setBoneTree | `<id>.setBoneTree(id)` | string id | void |
| setEditMode | `<id>.setEditMode(mode)` | string mode | void |
| setJointSpheresVisible | `<id>.setJointSpheresVisible(visible)` | bool visible | void |
| setMode | `<id>.setMode(mode)` | string mode | void |
| setSceneCharacterPos | `<id>.setSceneCharacterPos(index, x, y, z)` | int index, float x, float y, float z | void |
| setSceneCharacterRot | `<id>.setSceneCharacterRot(index, x, y, z)` | int index, float x, float y, float z | void |
| setSceneCharacterScale | `<id>.setSceneCharacterScale(index, x, y, z)` | int index, float x, float y, float z | void |
| setSceneCharacterVisible | `<id>.setSceneCharacterVisible(index, visible)` | int index, bool visible | void |
| setScenePropPos | `<id>.setScenePropPos(index, x, y, z)` | int index, float x, float y, float z | void |
| setScenePropRot | `<id>.setScenePropRot(index, x, y, z)` | int index, float x, float y, float z | void |
| setScenePropScale | `<id>.setScenePropScale(index, x, y, z)` | int index, float x, float y, float z | void |
| setScenePropVisible | `<id>.setScenePropVisible(index, visible)` | int index, bool visible | void |
| setSelectedBoneConstraint | `<id>.setSelectedBoneConstraint(minX, maxX, minY, maxY, minZ, maxZ)` | float minX, float maxX, float minY, float maxY, float minZ, float maxZ | void |
| setSelectedBoneRot | `<id>.setSelectedBoneRot(x, y, z)` | float x, float y, float z | void |
| setTransformSpace | `<id>.setTransformSpace(space)` | string space | void |
| set_show_bone_tree | `<id>.setShowBoneTree(value)` | bool value | void |
| startExportFrameRangePng | `<id>.startExportFrameRangePng(frameFrom, frameTo, outputDirectory)` | int frameFrom, int frameTo, string outputDirectory | bool |

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

