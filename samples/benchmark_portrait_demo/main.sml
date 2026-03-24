Window {
    id: mainWindow
    title: "SMS Native Benchmark"
    minSize: 420, 820
    size: 420, 820

    VBoxContainer {
        anchors: left | top | right | bottom
        padding: 18
        spacing: 10

        Label {
            id: titleLabel
            text: "SMS vs Kotlin (Reference)"
            fontSize: 20
        }

        Label {
            id: statusLabel
            text: "Tap a benchmark button."
            fontSize: 14
        }

        ProgressBar {
            id: runProgress
            minValue: 0
            maxValue: 100
            value: 0
        }

        Label {
            id: lblSms
            text: "SMS Native Score"
        }

        ProgressBar {
            id: barSms
            minValue: 0
            maxValue: 100
            value: 0
        }

        Label {
            id: lblKotlin
            text: "Kotlin Compose Baseline"
        }

        ProgressBar {
            id: barKotlin
            minValue: 0
            maxValue: 100
            value: 100
        }

        Label {
            id: resultLabel
            text: "Result: -"
        }

        HBoxContainer {
            spacing: 8
            Button {
                id: btnCpu
                text: "CPU"
                sizeFlagsHorizontal: expandFill
            }
            Button {
                id: btnDispatch
                text: "Dispatch"
                sizeFlagsHorizontal: expandFill
            }
        }

        HBoxContainer {
            spacing: 8
            Button {
                id: btnFrame
                text: "Frame"
                sizeFlagsHorizontal: expandFill
            }
            Button {
                id: btnMixed
                text: "Mixed"
                sizeFlagsHorizontal: expandFill
            }
        }
    }
}
