SplashScreen {
    id: splashScreen
    size: 800, 520
    duration: 600
    loadOnReady: "main.sml"

    VBoxContainer {
        anchors: left | top | right | bottom

        Control { sizeFlagsVertical: expandFill }

        Label {
            text: "Forge Theme Demo"
            fontSize: 18
            sizeFlagsHorizontal: shrinkCenter
        }

        Control { sizeFlagsVertical: expandFill }
    }
}
