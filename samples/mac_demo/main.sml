Window {
    id: mainWindow
    title: "Mac Demo"
    minSize: 640, 400
    size: 800, 500

    VBoxContainer {
        anchors: left | top | right | bottom
        padding: 40, 40, 40, 40

        Label {
            id: heading
            text: "Hello from Forge on macOS"
            fontSize: 24
        }

        Label {
            id: subline
            text: "This app was built with forgecli build mac."
            fontSize: 14
        }

        Control { sizeFlagsVertical: expandFill }

        Button {
            id: btnQuit
            text: "Quit"
            sizeFlagsHorizontal: shrinkCenter
        }
    }
}
