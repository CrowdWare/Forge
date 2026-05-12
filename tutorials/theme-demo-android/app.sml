SplashScreen {
    id: splashScreen
    size: 360, 640
    duration: 600
    loadOnReady: "main.sml"

    VBoxContainer {
        anchors: left | top | right | bottom

        Control { sizeFlagsVertical: expandFill }

        Label {
            text: "Forge Theme Demo"
            fontSize: 20
            sizeFlagsHorizontal: shrinkCenter
        }

        Control { sizeFlagsVertical: expandFill }
    }
}
