SplashScreen {
    id: splashScreen
    size: 640, 360
    duration: 300
    loadOnReady: "main.sml"

    VBoxContainer {
        anchors: left | top | right | bottom

        Control { sizeFlagsVertical: expandFill }

        Label {
            text: "Loading AOT Smoke Demo..."
            sizeFlagsHorizontal: shrinkCenter
        }

        Control { sizeFlagsVertical: expandFill }
    }
}
