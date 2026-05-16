Window {
    id: mainWindow
    title: "Forge Theme Demo"
    minSize: 640, 400
    size: 800, 520

    VBoxContainer {
        anchors: left | top | right | bottom

        Panel {
            id: headerBar
            height: 60
            bgColor: @Colors.headerBg

            MarginContainer {
                anchors: left | top | right | bottom
                paddingLeft: 24
                paddingRight: 24

                HBoxContainer {
                    spacing: 12

                    Label {
                        id: appTitle
                        text: "Forge Theme Demo"
                        fontSize: 20
                        color: @Colors.titleText
                        sizeFlagsVertical: shrinkCenter
                        sizeFlagsHorizontal: expandFill
                    }

                    Button {
                        id: btnTheme
                        text: "Dark Mode"
                        width: 110
                        sizeFlagsVertical: shrinkCenter
                    }
                }
            }
        }

        Box {
            id: contentArea
            sizeFlagsVertical: expandFill
            anchors: left | right
            bgColor: @Colors.contentBg

            ScrollContainer {
                anchors: left | top | right | bottom

                VBoxContainer {
                    sizeFlagsHorizontal: expandFill
                    spacing: 0

                    Control {
                        sizeFlagsVertical: shrinkCenter
                        height: 48
                    }

                    Box {
                        id: card
                        sizeFlagsHorizontal: shrinkCenter
                        width: 420
                        bgColor: @Colors.cardBg
                        borderRadius: 12
                        padding: 32, 32, 32, 32

                        VBoxContainer {
                            spacing: 20

                            Label {
                                id: cardTitle
                                text: "Hello from Forge"
                                fontSize: 22
                                color: @Colors.titleText
                            }

                            Label {
                                id: cardBody
                                text: "SML + SMS. One codebase. Mac and Android."
                                fontSize: 14
                                color: @Colors.bodyText
                            }

                            Box {
                                id: animBox
                                width: 72
                                height: 72
                                borderRadius: 36
                                bgColor: @Colors.animBoxColor
                                sizeFlagsHorizontal: shrinkCenter
                            }

                            Button {
                                id: btnAnimate
                                text: "Animate"
                                sizeFlagsHorizontal: shrinkCenter
                            }
                        }
                    }

                }
            }
        }
    }
}

