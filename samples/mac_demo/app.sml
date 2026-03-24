SplashScreen {
    id: splashScreen
    size: 640, 360
    duration: 800
    loadOnReady: "main.sml"

    VBoxContainer {
        anchors: left | top | right | bottom

        Control { sizeFlagsVertical: expandFill }

        Label {
            text: "Loading Mac Demo..."
            sizeFlagsHorizontal: shrinkCenter
        }

        Control { sizeFlagsVertical: expandFill }
    }
}
