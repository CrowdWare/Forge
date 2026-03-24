Window {
    id: mainWindow
    title: "AOT Smoke Demo"
    size: 640, 360

    VBoxContainer {
        anchors: left | top | right | bottom
        padding: 20
        spacing: 8

        Label {
            id: lblTitle
            text: "SMS AOT Smoke Test"
        }

        Label {
            id: lblHint
            text: "Check terminal output for AOT activation and green log output."
        }
    }
}
