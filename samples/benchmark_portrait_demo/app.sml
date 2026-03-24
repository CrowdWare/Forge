SplashScreen {
    id: splashScreen
    size: 420, 820
    duration: 250
    loadOnReady: "main.sml"

    VBoxContainer {
        anchors: left | top | right | bottom
        padding: 20
        spacing: 8

        Control { sizeFlagsVertical: expandFill }

        Label {
            text: "Loading Benchmark Demo..."
            sizeFlagsHorizontal: shrinkCenter
        }

        Control { sizeFlagsVertical: expandFill }
    }
}
