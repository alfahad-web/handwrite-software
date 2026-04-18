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
    Component.onCompleted: Qt.callLater(function () {
        if (writerController.viewMode === "typing")
            typeArea.forceActiveFocus()
    })

    readonly property color settingsTitleColor: "#1e3a8a"

    Connections {
        target: writerController
        function onLineHeightCollisionWarning() {
            lineHeightWarnDialog.open()
        }
    }

    Dialog {
        id: lineHeightWarnDialog
        parent: Overlay.overlay
        anchors.centerIn: parent
        modal: true
        title: "Layout warning"
        standardButtons: Dialog.Ok
        width: Math.min(420, root.width - 48)
        contentItem: Label {
            text: "After scaling, at least one glyph is taller than the line height. Lines may overlap or collide. Reduce the font unit scale or increase line height."
            wrapMode: Text.Wrap
            color: "#dc2626"
            width: parent ? parent.width : implicitWidth
        }
    }

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

            Connections {
                target: writerController
                function onViewModeChanged() {
                    if (writerController.viewMode === "typing")
                        Qt.callLater(function () { typeArea.forceActiveFocus() })
                }
            }

            TextArea {
                id: typeArea
                anchors.fill: parent
                visible: writerController.viewMode === "typing"
                z: writerController.viewMode === "typing" ? 2 : 0
                wrapMode: TextArea.Wrap
                selectByMouse: true
                selectByKeyboard: true
                cursorVisible: true
                font.pixelSize: 15
                color: "#18181b"
                selectedTextColor: "#fafafa"
                selectionColor: "#2563eb"
                placeholderTextColor: "#71717a"
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
                background: Rectangle {
                    color: "#fafafa"
                    border.color: "#e4e4e7"
                    border.width: 1
                }
                leftPadding: 12
                rightPadding: 12
                topPadding: 10
                bottomPadding: 10
            }

            Flickable {
                id: handFlick
                anchors.fill: parent
                visible: writerController.viewMode === "handwriting"
                z: writerController.viewMode === "handwriting" ? 2 : 0
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

                Label { text: "Feed rate (cm/s)"; font.bold: true; color: root.settingsTitleColor }
                SpinBox {
                    from: 1
                    to: 50000
                    stepSize: 1
                    editable: true
                    wheelEnabled: true
                    value: Math.round(writerController.settings.feedRateCmPerS * 1000)
                    onValueModified: writerController.settings.feedRateCmPerS = value / 1000.0
                }

                Label { text: "Page width (mm)"; font.bold: true; color: root.settingsTitleColor }
                SpinBox {
                    from: 100
                    to: 5000000
                    stepSize: 100
                    editable: true
                    wheelEnabled: true
                    value: Math.round(writerController.settings.pageWidthCm * 10 * 1000)
                    onValueModified: writerController.settings.pageWidthCm = value / 10000.0
                }

                Label { text: "Page height (mm)"; font.bold: true; color: root.settingsTitleColor }
                SpinBox {
                    from: 100
                    to: 5000000
                    stepSize: 100
                    editable: true
                    wheelEnabled: true
                    value: Math.round(writerController.settings.pageHeightCm * 10 * 1000)
                    onValueModified: writerController.settings.pageHeightCm = value / 10000.0
                }

                Label { text: "Left margin (mm)"; font.bold: true; color: root.settingsTitleColor }
                SpinBox {
                    from: 0
                    to: 5000000
                    stepSize: 100
                    editable: true
                    wheelEnabled: true
                    value: Math.round(writerController.settings.leftMarginCm * 10 * 1000)
                    onValueModified: writerController.settings.leftMarginCm = value / 10000.0
                }

                Label { text: "Right margin (mm)"; font.bold: true; color: root.settingsTitleColor }
                SpinBox {
                    from: 0
                    to: 5000000
                    stepSize: 100
                    editable: true
                    wheelEnabled: true
                    value: Math.round(writerController.settings.rightMarginCm * 10 * 1000)
                    onValueModified: writerController.settings.rightMarginCm = value / 10000.0
                }

                Label { text: "Vertical gap (mm)"; font.bold: true; color: root.settingsTitleColor }
                SpinBox {
                    from: 0
                    to: 5000000
                    stepSize: 100
                    editable: true
                    wheelEnabled: true
                    value: Math.round(writerController.settings.verticalGapCm * 10 * 1000)
                    onValueModified: writerController.settings.verticalGapCm = value / 10000.0
                }

                Label { text: "hx (mm)"; font.bold: true; color: root.settingsTitleColor }
                SpinBox {
                    from: 0
                    to: 5000000
                    stepSize: 100
                    editable: true
                    wheelEnabled: true
                    value: Math.round(writerController.settings.hxCm * 10 * 1000)
                    onValueModified: writerController.settings.hxCm = value / 10000.0
                }

                Label { text: "hy (mm)"; font.bold: true; color: root.settingsTitleColor }
                SpinBox {
                    from: 0
                    to: 5000000
                    stepSize: 100
                    editable: true
                    wheelEnabled: true
                    value: Math.round(writerController.settings.hyCm * 10 * 1000)
                    onValueModified: writerController.settings.hyCm = value / 10000.0
                }

                Label { text: "Line height (mm)"; font.bold: true; color: root.settingsTitleColor }
                SpinBox {
                    from: 100
                    to: 5000000
                    stepSize: 100
                    editable: true
                    wheelEnabled: true
                    value: Math.round(writerController.settings.lineHeightCm * 10 * 1000)
                    onValueModified: writerController.settings.lineHeightCm = value / 10000.0
                }

                Label { text: "Font unit → mm scale"; font.bold: true; color: root.settingsTitleColor }
                Label {
                    text: "Multiply coordinates from .txt by this (µm→mm use 0.001)"
                    wrapMode: Text.Wrap
                    Layout.fillWidth: true
                    color: "#3f3f46"
                }
                SpinBox {
                    from: 1
                    to: 1000000000
                    stepSize: 1000
                    editable: true
                    wheelEnabled: true
                    value: Math.round(writerController.settings.fontUnitToCm * 10 * 1000000)
                    onValueModified: writerController.settings.fontUnitToCm = value / 10000000.0
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
