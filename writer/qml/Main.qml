import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Writer 1.0

ApplicationWindow {
    id: root
    visible: true
    width: 1280
    height: 840
    minimumWidth: 900
    minimumHeight: 640
    title: "CNC handwriting preview"
    color: "#f4f4f5"

    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            spacing: 8
            Button {
                text: "Typing"
                highlighted: writerController.viewMode === "typing"
                onClicked: writerController.viewMode = "typing"
            }
            Button {
                text: "Handwriting"
                highlighted: writerController.viewMode === "handwriting"
                onClicked: writerController.viewMode = "handwriting"
            }
            ToolSeparator {}
            Button {
                text: "Select font folder"
                onClicked: writerController.pickFontFolder()
            }
            Button {
                text: "Settings"
                checkable: true
                checked: writerController.settingsOpen
                onClicked: writerController.settingsOpen = !writerController.settingsOpen
            }
            Button {
                text: writerController.runActive ? "Stop" : "Run"
                onClicked: writerController.runActive ? writerController.stopRun() : writerController.startRun()
            }
            Item { Layout.fillWidth: true }
            Label {
                text: writerController.fontStatus
                elide: Text.ElideRight
                Layout.maximumWidth: 360
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            TextArea {
                id: typeArea
                anchors.fill: parent
                visible: writerController.viewMode === "typing"
                wrapMode: TextArea.Wrap
                selectByMouse: true
                property bool docSync: false
                text: writerController.document.text
                onTextChanged: {
                    if (docSync) return
                    docSync = true
                    writerController.document.text = text
                    docSync = false
                }
                Connections {
                    target: writerController.document
                    function onTextChanged() {
                        if (typeArea.text === writerController.document.text) return
                        typeArea.docSync = true
                        typeArea.text = writerController.document.text
                        typeArea.docSync = false
                    }
                }
                background: Rectangle { color: "#fafafa" }
            }

            Flickable {
                id: handFlick
                anchors.fill: parent
                visible: writerController.viewMode === "handwriting"
                clip: true
                contentWidth: width
                contentHeight: handCanvas.implicitHeight
                boundsBehavior: Flickable.StopAtBounds

                HandwritingCanvasItem {
                    id: handCanvas
                    width: handFlick.width
                    height: implicitHeight
                    implicitHeight: 400
                    controller: writerController
                }
            }
        }

        Rectangle {
            Layout.preferredWidth: 260
            Layout.fillHeight: true
            color: "#fafafa"
            border.color: "#d4d4d8"
            ScrollView {
                anchors.fill: parent
                anchors.margins: 6
                TextArea {
                    id: gcodePlaceholder
                    readOnly: true
                    wrapMode: TextArea.Wrap
                    text: ""
                    placeholderText: "G-code / program output will appear here."
                    background: Item {}
                }
            }
        }
    }

    Rectangle {
        id: settingsDim
        anchors.fill: parent
        visible: writerController.settingsOpen
        color: "#00000055"
        z: 100
        MouseArea {
            anchors.fill: parent
            onClicked: writerController.settingsOpen = false
        }
    }

    Rectangle {
        id: settingsPanel
        z: 101
        visible: writerController.settingsOpen
        anchors.centerIn: parent
        width: Math.min(480, root.width - 40)
        height: Math.min(520, root.height - 40)
        radius: 8
        border.color: "#d4d4d8"
        color: "#ffffff"

        ScrollView {
            anchors.fill: parent
            anchors.margins: 12
            ColumnLayout {
                width: settingsPanel.width - 36
                spacing: 10

                Label { text: "Feed rate (cm/s)"; font.bold: true }
                SpinBox {
                    from: 1
                    to: 50000
                    stepSize: 1
                    editable: true
                    value: Math.round(writerController.settings.feedRateCmPerS * 1000)
                    onValueModified: writerController.settings.feedRateCmPerS = value / 1000.0
                }

                Label { text: "Page width (cm)"; font.bold: true }
                SpinBox {
                    from: 10
                    to: 500000
                    value: Math.round(writerController.settings.pageWidthCm * 1000)
                    onValueModified: writerController.settings.pageWidthCm = value / 1000.0
                }

                Label { text: "Page height (cm)"; font.bold: true }
                SpinBox {
                    from: 10
                    to: 500000
                    value: Math.round(writerController.settings.pageHeightCm * 1000)
                    onValueModified: writerController.settings.pageHeightCm = value / 1000.0
                }

                Label { text: "Left margin (cm)"; font.bold: true }
                SpinBox {
                    from: 0
                    to: 500000
                    value: Math.round(writerController.settings.leftMarginCm * 1000)
                    onValueModified: writerController.settings.leftMarginCm = value / 1000.0
                }

                Label { text: "Right margin (cm)"; font.bold: true }
                SpinBox {
                    from: 0
                    to: 500000
                    value: Math.round(writerController.settings.rightMarginCm * 1000)
                    onValueModified: writerController.settings.rightMarginCm = value / 1000.0
                }

                Label { text: "Vertical gap (cm)"; font.bold: true }
                SpinBox {
                    from: 0
                    to: 500000
                    value: Math.round(writerController.settings.verticalGapCm * 1000)
                    onValueModified: writerController.settings.verticalGapCm = value / 1000.0
                }

                Label { text: "hx (cm)"; font.bold: true }
                SpinBox {
                    from: 0
                    to: 500000
                    value: Math.round(writerController.settings.hxCm * 1000)
                    onValueModified: writerController.settings.hxCm = value / 1000.0
                }

                Label { text: "hy (cm)"; font.bold: true }
                SpinBox {
                    from: 0
                    to: 500000
                    value: Math.round(writerController.settings.hyCm * 1000)
                    onValueModified: writerController.settings.hyCm = value / 1000.0
                }

                Label { text: "Line height (cm)"; font.bold: true }
                SpinBox {
                    from: 10
                    to: 500000
                    value: Math.round(writerController.settings.lineHeightCm * 1000)
                    onValueModified: writerController.settings.lineHeightCm = value / 1000.0
                }

                Label { text: "Font unit → cm scale"; font.bold: true }
                Label {
                    text: "Multiply coordinates from .txt by this (µm→cm use 0.0001)"
                    wrapMode: Text.Wrap
                    Layout.fillWidth: true
                }
                SpinBox {
                    from: 1
                    to: 100000000
                    value: Math.round(writerController.settings.fontUnitToCm * 1000000)
                    onValueModified: writerController.settings.fontUnitToCm = value / 1000000.0
                }

                RowLayout {
                    Layout.fillWidth: true
                    Button {
                        text: "Save settings"
                        onClicked: writerController.settings.save()
                    }
                    Button {
                        text: "Close"
                        onClicked: writerController.settingsOpen = false
                    }
                }
            }
        }
    }
}
