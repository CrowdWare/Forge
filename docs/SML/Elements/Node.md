# Node

> Note: This is a base class included for inheritance documentation. It is **not** an SML element.

## Inheritance

[Node](Node.md) → [Object](Object.md)

## Derived Classes

Classes listed below inherit from `Node`.

### Direct subclasses

- [AnimationMixer](AnimationMixer.md)
- [AudioStreamPlayer](AudioStreamPlayer.md)
- [CanvasItem](CanvasItem.md)
- [CanvasLayer](CanvasLayer.md)
- [EditorFileSystem](EditorFileSystem.md)
- [EditorPlugin](EditorPlugin.md)
- [EditorResourcePreview](EditorResourcePreview.md)
- [ForgeRunnerNativeMain](ForgeRunnerNativeMain.md)
- [HTTPRequest](HTTPRequest.md)
- [InstancePlaceholder](InstancePlaceholder.md)
- [MissingNode](MissingNode.md)
- [MultiplayerSpawner](MultiplayerSpawner.md)
- [MultiplayerSynchronizer](MultiplayerSynchronizer.md)
- [NavigationAgent2D](NavigationAgent2D.md)
- [NavigationAgent3D](NavigationAgent3D.md)
- [Node3D](Node3D.md)
- [ResourcePreloader](ResourcePreloader.md)
- [ShaderGlobalsOverride](ShaderGlobalsOverride.md)
- [StatusIndicator](StatusIndicator.md)
- [Timer](Timer.md)
- [Viewport](Viewport.md)
- [WorldEnvironment](WorldEnvironment.md)

## Properties

This page lists **only properties declared by `Node`**.
Inherited properties are documented in: [Object](Object.md)

| Godot Property | SML Property | Type | Default |
|-|-|-|-|
| auto_translate_mode | autoTranslateMode | int | - |
| editor_description | editorDescription | string | - |
| physics_interpolation_mode | physicsInterpolationMode | int | - |
| process_mode | processMode | int | - |
| process_physics_priority | processPhysicsPriority | int | - |
| process_priority | processPriority | int | - |
| process_thread_group | processThreadGroup | int | - |
| process_thread_group_order | processThreadGroupOrder | int | - |
| process_thread_messages | processThreadMessages | int | - |

## Events

This page lists **only signals declared by `Node`**.
Inherited signals are documented in: [Object](Object.md)

| Godot Signal | SMS Event | Params |
|-|-|-|
| child_entered_tree | `on <id>.childEnteredTree(node) { ... }` | Object node |
| child_exiting_tree | `on <id>.childExitingTree(node) { ... }` | Object node |
| child_order_changed | `on <id>.childOrderChanged() { ... }` | - |
| editor_description_changed | `on <id>.editorDescriptionChanged(node) { ... }` | Object node |
| editor_state_changed | `on <id>.editorStateChanged() { ... }` | - |
| ready | `on <id>.ready() { ... }` | - |
| renamed | `on <id>.renamed() { ... }` | - |
| replacing_by | `on <id>.replacingBy(node) { ... }` | Object node |
| tree_entered | `on <id>.treeEntered() { ... }` | - |
| tree_exited | `on <id>.treeExited() { ... }` | - |
| tree_exiting | `on <id>.treeExiting() { ... }` | - |

## Runtime Actions

This page lists **callable methods declared by `Node`**.
Inherited actions are documented in: [Object](Object.md)

| Godot Method | SMS Call | Params | Returns |
|-|-|-|-|
| can_auto_translate | `<id>.canAutoTranslate()` | - | bool |
| can_process | `<id>.canProcess()` | - | bool |
| create_tween | `<id>.createTween()` | - | Object |
| duplicate | `<id>.duplicate(flags)` | int flags | Object |
| find_child | `<id>.findChild(pattern, recursive, owned)` | string pattern, bool recursive, bool owned | Object |
| find_children | `<id>.findChildren(pattern, type, recursive, owned)` | string pattern, string type, bool recursive, bool owned | Variant |
| find_parent | `<id>.findParent(pattern)` | string pattern | Object |
| get_accessibility_element | `<id>.getAccessibilityElement()` | - | Variant |
| get_child | `<id>.getChild(idx, includeInternal)` | int idx, bool includeInternal | Object |
| get_child_count | `<id>.getChildCount(includeInternal)` | bool includeInternal | int |
| get_children | `<id>.getChildren(includeInternal)` | bool includeInternal | Variant |
| get_groups | `<id>.getGroups()` | - | Variant |
| get_index | `<id>.getIndex(includeInternal)` | bool includeInternal | int |
| get_last_exclusive_window | `<id>.getLastExclusiveWindow()` | - | Object |
| get_multiplayer_authority | `<id>.getMultiplayerAuthority()` | - | int |
| get_node_rpc_config | `<id>.getNodeRpcConfig()` | - | void |
| get_orphan_node_ids | `<id>.getOrphanNodeIds()` | - | Variant |
| get_parent | `<id>.getParent()` | - | Object |
| get_path | `<id>.getPath()` | - | Variant |
| get_physics_process_delta_time | `<id>.getPhysicsProcessDeltaTime()` | - | float |
| get_physics_process_priority | `<id>.getPhysicsProcessPriority()` | - | int |
| get_process_delta_time | `<id>.getProcessDeltaTime()` | - | float |
| get_scene_instance_load_placeholder | `<id>.getSceneInstanceLoadPlaceholder()` | - | bool |
| get_tree | `<id>.getTree()` | - | Object |
| get_tree_string | `<id>.getTreeString()` | - | string |
| get_tree_string_pretty | `<id>.getTreeStringPretty()` | - | string |
| get_viewport | `<id>.getViewport()` | - | Object |
| get_window | `<id>.getWindow()` | - | Object |
| is_displayed_folded | `<id>.isDisplayedFolded()` | - | bool |
| is_inside_tree | `<id>.isInsideTree()` | - | bool |
| is_multiplayer_authority | `<id>.isMultiplayerAuthority()` | - | bool |
| is_node_ready | `<id>.isNodeReady()` | - | bool |
| is_part_of_edited_scene | `<id>.isPartOfEditedScene()` | - | bool |
| is_physics_interpolated | `<id>.isPhysicsInterpolated()` | - | bool |
| is_physics_interpolated_and_enabled | `<id>.isPhysicsInterpolatedAndEnabled()` | - | bool |
| is_physics_processing | `<id>.isPhysicsProcessing()` | - | bool |
| is_physics_processing_internal | `<id>.isPhysicsProcessingInternal()` | - | bool |
| is_processing | `<id>.isProcessing()` | - | bool |
| is_processing_input | `<id>.isProcessingInput()` | - | bool |
| is_processing_internal | `<id>.isProcessingInternal()` | - | bool |
| is_processing_shortcut_input | `<id>.isProcessingShortcutInput()` | - | bool |
| is_processing_unhandled_input | `<id>.isProcessingUnhandledInput()` | - | bool |
| is_processing_unhandled_key_input | `<id>.isProcessingUnhandledKeyInput()` | - | bool |
| notify_deferred_thread_group | `<id>.notifyDeferredThreadGroup(what)` | int what | void |
| notify_thread_safe | `<id>.notifyThreadSafe(what)` | int what | void |
| print_orphan_nodes | `<id>.printOrphanNodes()` | - | void |
| print_tree | `<id>.printTree()` | - | void |
| print_tree_pretty | `<id>.printTreePretty()` | - | void |
| propagate_notification | `<id>.propagateNotification(what)` | int what | void |
| queue_accessibility_update | `<id>.queueAccessibilityUpdate()` | - | void |
| queue_free | `<id>.queueFree()` | - | void |
| request_ready | `<id>.requestReady()` | - | void |
| reset_physics_interpolation | `<id>.resetPhysicsInterpolation()` | - | void |
| set_display_folded | `<id>.setDisplayFolded(fold)` | bool fold | void |
| set_multiplayer_authority | `<id>.setMultiplayerAuthority(id, recursive)` | int id, bool recursive | void |
| set_physics_process | `<id>.setPhysicsProcess(enable)` | bool enable | void |
| set_physics_process_internal | `<id>.setPhysicsProcessInternal(enable)` | bool enable | void |
| set_physics_process_priority | `<id>.setPhysicsProcessPriority(priority)` | int priority | void |
| set_process | `<id>.setProcess(enable)` | bool enable | void |
| set_process_input | `<id>.setProcessInput(enable)` | bool enable | void |
| set_process_internal | `<id>.setProcessInternal(enable)` | bool enable | void |
| set_process_shortcut_input | `<id>.setProcessShortcutInput(enable)` | bool enable | void |
| set_process_unhandled_input | `<id>.setProcessUnhandledInput(enable)` | bool enable | void |
| set_process_unhandled_key_input | `<id>.setProcessUnhandledKeyInput(enable)` | bool enable | void |
| set_scene_instance_load_placeholder | `<id>.setSceneInstanceLoadPlaceholder(loadPlaceholder)` | bool loadPlaceholder | void |
| set_translation_domain_inherited | `<id>.setTranslationDomainInherited()` | - | void |
| update_configuration_warnings | `<id>.updateConfigurationWarnings()` | - | void |
