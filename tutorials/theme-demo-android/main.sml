Window {
    id: mainWindow
    title: "Forge Theme Demo"
    size: 360, 640

    VBoxContainer {
        anchors: left | top | right | bottom

        Box {
            id: headerBar
            height: 72
            anchors: left | right
            bgColor: @Colors.headerBg
            padding: 0, 20, 0, 20

            Row {
                anchors: left | top | right | bottom
                spacing: 12

                Label {
                    id: appTitle
                    text: "Forge Theme Demo"
                    fontSize: 22
                    color: @Colors.titleText
                    sizeFlagsVertical: shrinkCenter
                    sizeFlagsHorizontal: expandFill
                }

                Button {
                    id: btnTheme
                    text: "🌙"
                    width: 56
                    height: 56
                    fontSize: 22
                    sizeFlagsVertical: shrinkCenter
                }
            }
        }

        Box {
            id: contentArea
            sizeFlagsVertical: expandFill
            anchors: left | right
            bgColor: @Colors.contentBg
            padding: 24, 0, 24, 0

            VBoxContainer {
                anchors: left | top | right | bottom
                spacing: 0

                Control { sizeFlagsVertical: expandFill }

                Box {
                    id: card
                    anchors: left | right
                    bgColor: @Colors.cardBg
                    borderRadius: 16
                    padding: 28, 28, 28, 28

                    VBoxContainer {
                        spacing: 24

                        Label {
                            id: cardTitle
                            text: "Hello from Forge"
                            fontSize: 26
                            color: @Colors.titleText
                        }

                        Label {
                            id: cardBody
                            text: "SML + SMS. One codebase. Mac and Android."
                            fontSize: 16
                            color: @Colors.bodyText
                        }

                        Box {
                            id: animBox
                            width: 88
                            height: 88
                            borderRadius: 44
                            bgColor: @Colors.animBoxColor
                            sizeFlagsHorizontal: shrinkCenter
                        }

                        Button {
                            id: btnAnimate
                            text: "Animate"
                            height: 64
                            fontSize: 20
                            sizeFlagsHorizontal: expandFill
                        }
                    }
                }

                Control { sizeFlagsVertical: expandFill }
            }
        }
    }
}
