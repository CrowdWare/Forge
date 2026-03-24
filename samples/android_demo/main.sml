Window {
    id: mainWindow
    title: "Android Demo"
    size: 360, 640

    VBoxContainer {
        anchors: left | top | right | bottom
        padding: 32, 32, 32, 32

        Label {
            id: heading
            text: "Hello from Forge on Android"
            fontSize: 40
        }

        Label {
            id: subline
            text: "Vertical slice - build with forgecli build android."
            fontSize: 25
        }

        Control { sizeFlagsVertical: expandFill }

        Button {
            id: btnTest
            width: 300
            height: 100
            text: "Tap me"
            fontSize: 25
            sizeFlagsHorizontal: shrinkCenter
        }
    }
}
