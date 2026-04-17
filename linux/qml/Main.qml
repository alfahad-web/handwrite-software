import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Handwrite 1.0

ApplicationWindow {
    id: root
    visible: true
    width: 1280
    height: 840
    minimumWidth: 900
    minimumHeight: 640
    title: "Handwrite Qt"
    color: "#f4f4f5"
    property int debugMoveCount: 0
    Component.onCompleted: {
        console.log("[qml] Main loaded. toolMode=", editorStoreModel.toolMode,
                    "zoom=", editorStoreModel.zoom, "strokePx=", editorStoreModel.strokePx)
    }
    onClosing: appController.setBoardCursorActive(false)

    header: Frame {
        background: Rectangle { color: "#fafafa"; border.color: "#d4d4d8" }
        padding: 10
        RowLayout {
            anchors.fill: parent
            spacing: 10

            Label { text: "Stroke" }
            SpinBox {
                from: 1; to: 200
                value: editorStoreModel.strokePx
                onValueModified: editorStoreModel.setStrokePx(value)
            }

            Label { text: "Gap (um)" }
            SpinBox {
                from: 1; to: 200000
                value: editorStoreModel.captureGapUm
                editable: true
                onValueModified: editorStoreModel.setCaptureGapUm(value)
            }

            Button {
                text: editorStoreModel.openFileName.length > 0 ? editorStoreModel.openFileName : "Open/Create .txt"
                onClicked: appController.openOrCreateFile()
            }

            Button {
                text: "\u2713"
                enabled: appController.canWriteSelection()
                onClicked: appController.appendSelection()
            }

            Button {
                text: "\u2715"
                enabled: editorStoreModel.openFilePath.length > 0
                onClicked: appController.appendSelectionAndClose()
            }

            Button {
                text: "-"
                onClicked: editorStoreModel.zoomOut()
            }
            SpinBox {
                from: 10; to: 800
                value: editorStoreModel.zoom
                onValueModified: editorStoreModel.setZoom(value)
            }
            Button {
                text: "+"
                onClicked: editorStoreModel.zoomIn()
            }

            Button {
                text: editorStoreModel.toolMode === "select" ? "Selection On" : "Selection Off"
                highlighted: editorStoreModel.toolMode === "select"
                onClicked: editorStoreModel.toggleToolMode()
            }

            Item { Layout.fillWidth: true }

            Label {
                text: editorStoreModel.isDirty ? "Unsaved board" : "Saved board"
                color: "#1e293b"
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#f4f4f5"
            clip: true

            Flickable {
                anchors.fill: parent
                contentWidth: 3000 * editorStoreModel.zoom / 100
                contentHeight: 2000 * editorStoreModel.zoom / 100
                boundsBehavior: Flickable.StopAtBounds
                interactive: false

                CanvasItem {
                    id: boardCanvas
                    width: 3000 * editorStoreModel.zoom / 100
                    height: 2000 * editorStoreModel.zoom / 100
                    editorStore: editorStoreModel
                }

                MouseArea {
                    anchors.fill: boardCanvas
                    hoverEnabled: true
                    acceptedButtons: Qt.LeftButton
                    onEntered: {
                        console.log("[qml] MouseArea entered; toolMode=", editorStoreModel.toolMode)
                        if (editorStoreModel.toolMode === "draw") appController.setBoardCursorActive(true)
                    }
                    onExited: {
                        console.log("[qml] MouseArea exited")
                        appController.setBoardCursorActive(false)
                    }
                    onPressed: (mouse) => {
                        console.log("[qml] press", mouse.x, mouse.y, "button=", mouse.button)
                        boardCanvas.pointerDown(mouse.x, mouse.y, mouse.button)
                    }
                    onPositionChanged: (mouse) => {
                        root.debugMoveCount += 1
                        if ((root.debugMoveCount % 25) === 0) {
                            console.log("[qml] move#", root.debugMoveCount, mouse.x, mouse.y, "contains=", containsMouse)
                        }
                        boardCanvas.pointerMove(mouse.x, mouse.y)
                        if (containsMouse && editorStoreModel.toolMode === "draw") appController.setBoardCursorActive(true)
                    }
                    onReleased: (mouse) => {
                        console.log("[qml] release", mouse.x, mouse.y, "button=", mouse.button)
                        boardCanvas.pointerUp(mouse.x, mouse.y, mouse.button)
                        if (!containsMouse || editorStoreModel.toolMode !== "draw") appController.setBoardCursorActive(false)
                    }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            height: 30
            color: "#e2e8f0"
            Label {
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: 10
                text: appController.statusMessage.length > 0 ? appController.statusMessage : "Ready"
                color: "#1e293b"
            }
        }
    }
}
