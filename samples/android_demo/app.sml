SplashScreen {
    id: splashScreen
    size: 360, 640
    duration: 800
    loadOnReady: "main.sml"

    VBoxContainer {
        anchors: left | top | right | bottom

        Control { sizeFlagsVertical: expandFill }

        Label {
            text: "Loading..."
            sizeFlagsHorizontal: shrinkCenter
        }

        Control { sizeFlagsVertical: expandFill }
    }
}
