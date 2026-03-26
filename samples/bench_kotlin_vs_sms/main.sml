Window {
    id: mainWindow
    title: "Kotlin vs SMS"
    size: 360, 640

    VBoxContainer {
        anchors: left | top | right | bottom
        padding: 24, 24, 24, 24

        Label {
            id: heading
            text: "SMS Benchmark"
            fontSize: 30
        }

        Label {
            id: status
            text: "Ready"
            fontSize: 22
        }

        Button {
            id: btnRun
            width: 300
            height: 88
            text: "Run SMS"
            fontSize: 24
            sizeFlagsHorizontal: shrinkCenter
        }

        Control { height: 16 }

        Button {
            id: btnKotlin
            width: 300
            height: 88
            text: "Run Kotlin"
            fontSize: 24
            sizeFlagsHorizontal: shrinkCenter
        }

        Control { height: 16 }

        Button {
            id: btnSmsKiller
            width: 300
            height: 88
            text: "Killer SMS"
            fontSize: 24
            sizeFlagsHorizontal: shrinkCenter
        }

        Control { height: 16 }

        Button {
            id: btnKotlinKiller
            width: 300
            height: 88
            text: "Killer Kotlin"
            fontSize: 24
            sizeFlagsHorizontal: shrinkCenter
        }
    }
}
