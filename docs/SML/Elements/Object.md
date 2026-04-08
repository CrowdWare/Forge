# Object

> Note: This is a base class included for inheritance documentation. It is **not** an SML element.

## Inheritance

[Object](Object.md)

## Derived Classes

Classes listed below inherit from `Object`.

### Direct subclasses

- [AudioServer](AudioServer.md)
- [CameraServer](CameraServer.md)
- [ClassDB](ClassDB.md)
- [DisplayServer](DisplayServer.md)
- [EditorFileSystemDirectory](EditorFileSystemDirectory.md)
- [EditorInterface](EditorInterface.md)
- [EditorPaths](EditorPaths.md)
- [EditorSelection](EditorSelection.md)
- [EditorUndoRedoManager](EditorUndoRedoManager.md)
- [EditorVCSInterface](EditorVCSInterface.md)
- [Engine](Engine.md)
- [EngineDebugger](EngineDebugger.md)
- [FramebufferCacheRD](FramebufferCacheRD.md)
- [GDExtensionManager](GDExtensionManager.md)
- [Geometry2D](Geometry2D.md)
- [Geometry3D](Geometry3D.md)
- [GodotInstance](GodotInstance.md)
- [GodotSharp](GodotSharp.md)
- [IP](IP.md)
- [Input](Input.md)
- [InputMap](InputMap.md)
- [JNISingleton](JNISingleton.md)
- [JSONRPC](JSONRPC.md)
- [JavaClassWrapper](JavaClassWrapper.md)
- [JavaScriptBridge](JavaScriptBridge.md)
- [MainLoop](MainLoop.md)
- [ManagedCallableMiddleman](ManagedCallableMiddleman.md)
- [Marshalls](Marshalls.md)
- [MovieWriter](MovieWriter.md)
- [NativeMenu](NativeMenu.md)
- [NavigationMeshGenerator](NavigationMeshGenerator.md)
- [NavigationServer2D](NavigationServer2D.md)
- [NavigationServer2DManager](NavigationServer2DManager.md)
- [NavigationServer3D](NavigationServer3D.md)
- [NavigationServer3DManager](NavigationServer3DManager.md)
- [Node](Node.md)
- [OS](OS.md)
- [OpenXRExtensionWrapper](OpenXRExtensionWrapper.md)
- [OpenXRInteractionProfileMetadata](OpenXRInteractionProfileMetadata.md)
- [Performance](Performance.md)
- [PhysicsDirectBodyState2D](PhysicsDirectBodyState2D.md)
- [PhysicsDirectBodyState3D](PhysicsDirectBodyState3D.md)
- [PhysicsDirectSpaceState2D](PhysicsDirectSpaceState2D.md)
- [PhysicsDirectSpaceState3D](PhysicsDirectSpaceState3D.md)
- [PhysicsServer2D](PhysicsServer2D.md)
- [PhysicsServer2DManager](PhysicsServer2DManager.md)
- [PhysicsServer3D](PhysicsServer3D.md)
- [PhysicsServer3DManager](PhysicsServer3DManager.md)
- [PhysicsServer3DRenderingServerHandler](PhysicsServer3DRenderingServerHandler.md)
- [ProjectSettings](ProjectSettings.md)
- [RefCounted](RefCounted.md)
- [RenderData](RenderData.md)
- [RenderSceneData](RenderSceneData.md)
- [RenderingDevice](RenderingDevice.md)
- [RenderingServer](RenderingServer.md)
- [ResourceLoader](ResourceLoader.md)
- [ResourceSaver](ResourceSaver.md)
- [ResourceUID](ResourceUID.md)
- [ScriptLanguage](ScriptLanguage.md)
- [ShaderIncludeDB](ShaderIncludeDB.md)
- [TextServerManager](TextServerManager.md)
- [ThemeContext](ThemeContext.md)
- [ThemeDB](ThemeDB.md)
- [TileData](TileData.md)
- [Time](Time.md)
- [TranslationServer](TranslationServer.md)
- [TreeItem](TreeItem.md)
- [UndoRedo](UndoRedo.md)
- [UniformSetCacheRD](UniformSetCacheRD.md)
- [WorkerThreadPool](WorkerThreadPool.md)
- [XRServer](XRServer.md)
- [XRVRS](XRVRS.md)

## Properties

This page lists **only properties declared by `Object`**.

| Godot Property | SML Property | Type | Default |
|-|-|-|-|

## Events

This page lists **only signals declared by `Object`**.

| Godot Signal | SMS Event | Params |
|-|-|-|
| property_list_changed | `on <id>.propertyListChanged() { ... }` | - |
| script_changed | `on <id>.scriptChanged() { ... }` | - |

## Runtime Actions

This page lists **callable methods declared by `Object`**.

| Godot Method | SMS Call | Params | Returns |
|-|-|-|-|
| can_translate_messages | `<id>.canTranslateMessages()` | - | bool |
| cancel_free | `<id>.cancelFree()` | - | void |
| free | `<id>.free()` | - | void |
| get_class | `<id>.getClass()` | - | string |
| get_incoming_connections | `<id>.getIncomingConnections()` | - | Variant |
| get_instance_id | `<id>.getInstanceId()` | - | int |
| get_meta_list | `<id>.getMetaList()` | - | Variant |
| get_method_list | `<id>.getMethodList()` | - | Variant |
| get_property_list | `<id>.getPropertyList()` | - | Variant |
| get_script | `<id>.getScript()` | - | void |
| get_signal_list | `<id>.getSignalList()` | - | Variant |
| get_translation_domain | `<id>.getTranslationDomain()` | - | Variant |
| is_blocking_signals | `<id>.isBlockingSignals()` | - | bool |
| is_class | `<id>.isClass(class)` | string class | bool |
| is_queued_for_deletion | `<id>.isQueuedForDeletion()` | - | bool |
| notification | `<id>.notification(what, reversed)` | int what, bool reversed | void |
| notify_property_list_changed | `<id>.notifyPropertyListChanged()` | - | void |
| set_block_signals | `<id>.setBlockSignals(enable)` | bool enable | void |
| set_message_translation | `<id>.setMessageTranslation(enable)` | bool enable | void |
| to_string | `<id>.toString()` | - | string |
