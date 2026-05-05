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
    property bool boardPanMode: false
    property bool boardPointerPressed: false
    property real panLastX: 0
    property real panLastY: 0
    readonly property color headerLabelColor: "#1e3a8a"
    readonly property real boardBaseWidth: Math.max(3000, width * 3)
    readonly property real boardBaseHeight: Math.max(2000, height * 3)
    readonly property bool eraseBrushAppCursor: editorStoreModel.toolMode === "erase"
        || (editorStoreModel.toolMode === "draw" && editorStoreModel.drawStrokeEraseActive)
    Component.onCompleted: {
        console.log("[qml] Main loaded. toolMode=", editorStoreModel.toolMode,
                    "zoom=", editorStoreModel.zoom, "strokePx=", editorStoreModel.strokePx)
    }
    Connections {
        target: editorStoreModel
        function onToolModeChanged() {
            if (editorStoreModel.toolMode !== "draw") appController.setBoardCursorActive(false)
            if (root.eraseBrushAppCursor) {
                appController.setEraseCursorActive(true, editorStoreModel.eraseRadiusPx, editorStoreModel.zoom)
            } else {
                appController.setEraseCursorActive(false, editorStoreModel.eraseRadiusPx, editorStoreModel.zoom)
            }
        }
        function onDrawStrokeEraseActiveChanged() {
            if (root.eraseBrushAppCursor) {
                appController.setEraseCursorActive(true, editorStoreModel.eraseRadiusPx, editorStoreModel.zoom)
            } else {
                appController.setEraseCursorActive(false, editorStoreModel.eraseRadiusPx, editorStoreModel.zoom)
            }
        }
        function onEraseRadiusPxChanged() {
            if (root.eraseBrushAppCursor) {
                appController.setEraseCursorActive(true, editorStoreModel.eraseRadiusPx, editorStoreModel.zoom)
            }
        }
        function onZoomChanged() {
            if (root.eraseBrushAppCursor) {
                appController.setEraseCursorActive(true, editorStoreModel.eraseRadiusPx, editorStoreModel.zoom)
            }
        }
    }
    onClosing: appController.setBoardCursorActive(false)
    property string pendingSelectionAssignId: ""
    property string pendingAssignJoin: "N"

    header: Frame {
        background: Rectangle { color: "#fafafa"; border.color: "#d4d4d8" }
        padding: 10
        implicitHeight: controlsFlow.implicitHeight + (padding * 2)
        Flow {
            id: controlsFlow
            width: parent.width
            spacing: 10
            flow: Flow.LeftToRight

            Label { text: "Stroke"; color: root.headerLabelColor }
            SpinBox {
                from: 1; to: 200
                value: editorStoreModel.strokePx
                onValueModified: editorStoreModel.setStrokePx(value)
            }

            Label { text: "Gap (um)"; color: root.headerLabelColor }
            SpinBox {
                from: 1; to: 200000
                value: editorStoreModel.captureGapUm
                editable: true
                onValueModified: editorStoreModel.setCaptureGapUm(value)
            }

            Label { text: "Guide gap (px)"; color: root.headerLabelColor }
            SpinBox {
                from: 10; to: 1000
                value: editorStoreModel.guideLineGapPx
                editable: true
                onValueModified: editorStoreModel.setGuideLineGapPx(value)
            }

            Button {
                id: fileButton
                text: editorStoreModel.projectFileName.length > 0 ? editorStoreModel.projectFileName : "File"
                onClicked: fileMenu.open()
            }
            Menu {
                id: fileMenu
                y: fileButton.height
                MenuItem {
                    text: "New"
                    onTriggered: appController.newProject()
                }
                MenuItem {
                    text: "Open"
                    onTriggered: appController.openProject()
                }
                MenuItem {
                    text: "Save"
                    onTriggered: appController.saveProject()
                }
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
                onClicked: editorStoreModel.setToolMode("select")
            }
            Button {
                text: editorStoreModel.toolMode === "draw" ? "Draw On" : "Draw"
                highlighted: editorStoreModel.toolMode === "draw"
                onClicked: editorStoreModel.setToolMode("draw")
            }
            Button {
                text: root.eraseBrushAppCursor ? "Erase On" : "Erase"
                highlighted: root.eraseBrushAppCursor
                onClicked: {
                    if (editorStoreModel.toolMode === "draw") {
                        // WRITE accessor must be Q_INVOKABLE for call form; assignment always works.
                        editorStoreModel.drawStrokeEraseActive = !editorStoreModel.drawStrokeEraseActive
                    } else {
                        editorStoreModel.setToolMode("erase")
                    }
                }
            }
            Label { text: "Erase r(px)"; color: root.headerLabelColor }
            SpinBox {
                from: 1; to: 500
                value: editorStoreModel.eraseRadiusPx
                onValueModified: editorStoreModel.setEraseRadiusPx(value)
            }
            Button {
                text: "Delete Sel"
                enabled: editorStoreModel.hasSelectedSelection
                onClicked: appController.deleteSelectedSelection()
            }
            Button {
                text: "Generate Fonts"
                onClicked: appController.generateFonts()
            }

            Label {
                text: editorStoreModel.isDirty ? "Unsaved board" : "Saved board"
                color: root.headerLabelColor
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            id: boardViewport
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#f4f4f5"
            clip: true

            Flickable {
                id: boardFlick
                anchors.fill: parent
                contentWidth: root.boardBaseWidth * editorStoreModel.zoom / 100
                contentHeight: root.boardBaseHeight * editorStoreModel.zoom / 100
                boundsBehavior: Flickable.StopAtBounds
                flickableDirection: Flickable.HorizontalAndVerticalFlick
                // Keep panning controlled by explicit UI (scrollbars / custom pan mode)
                // so draw gestures are never interpreted as viewport drags.
                interactive: false
                clip: true
                ScrollBar.vertical: ScrollBar {
                    policy: ScrollBar.AlwaysOn
                    visible: true
                    interactive: true
                }
                ScrollBar.horizontal: ScrollBar {
                    policy: ScrollBar.AlwaysOn
                    visible: true
                    interactive: true
                }

                CanvasItem {
                    id: boardCanvas
                    width: root.boardBaseWidth * editorStoreModel.zoom / 100
                    height: root.boardBaseHeight * editorStoreModel.zoom / 100
                    editorStore: editorStoreModel
                    onSelectionDoubleClicked: (selectionId) => {
                        pendingSelectionAssignId = selectionId
                        assignDialog.open()
                    }
                }

                MouseArea {
                    anchors.fill: boardCanvas
                    hoverEnabled: true
                    acceptedButtons: Qt.LeftButton
                    onEntered: {
                        console.log("[qml] MouseArea entered; toolMode=", editorStoreModel.toolMode)
                        if (editorStoreModel.toolMode === "draw") {
                            appController.setEraseCursorActive(editorStoreModel.drawStrokeEraseActive, editorStoreModel.eraseRadiusPx, editorStoreModel.zoom)
                            appController.setBoardCursorActive(!editorStoreModel.drawStrokeEraseActive)
                        } else if (editorStoreModel.toolMode === "erase") {
                            appController.setBoardCursorActive(false)
                            appController.setEraseCursorActive(true, editorStoreModel.eraseRadiusPx, editorStoreModel.zoom)
                        } else {
                            appController.setBoardCursorActive(false)
                            appController.setEraseCursorActive(false, editorStoreModel.eraseRadiusPx, editorStoreModel.zoom)
                        }
                    }
                    onExited: {
                        console.log("[qml] MouseArea exited")
                        appController.setBoardCursorActive(false)
                        appController.setEraseCursorActive(false, editorStoreModel.eraseRadiusPx, editorStoreModel.zoom)
                        boardPointerPressed = false
                        boardPanMode = false
                    }
                    onPressed: (mouse) => {
                        console.log("[qml] press", mouse.x, mouse.y, "button=", mouse.button)
                        boardPointerPressed = true
                        if (boardPanMode) {
                            panLastX = mouse.x
                            panLastY = mouse.y
                            return
                        }
                        boardCanvas.pointerDown(mouse.x, mouse.y, mouse.button)
                    }
                    onPositionChanged: (mouse) => {
                        root.debugMoveCount += 1
                        if ((root.debugMoveCount % 25) === 0) {
                            console.log("[qml] move#", root.debugMoveCount, mouse.x, mouse.y, "contains=", containsMouse)
                        }
                        if (boardPanMode) {
                            const dx = mouse.x - panLastX
                            const dy = mouse.y - panLastY
                            boardFlick.contentX = Math.max(0, Math.min(boardFlick.contentWidth - boardFlick.width, boardFlick.contentX - dx))
                            boardFlick.contentY = Math.max(0, Math.min(boardFlick.contentHeight - boardFlick.height, boardFlick.contentY - dy))
                            panLastX = mouse.x
                            panLastY = mouse.y
                            return
                        }
                        if (boardPointerPressed || editorStoreModel.toolMode !== "draw") {
                            boardCanvas.pointerMove(mouse.x, mouse.y)
                        }
                        if (containsMouse && editorStoreModel.toolMode === "draw") {
                            appController.setEraseCursorActive(editorStoreModel.drawStrokeEraseActive, editorStoreModel.eraseRadiusPx, editorStoreModel.zoom)
                            appController.setBoardCursorActive(!editorStoreModel.drawStrokeEraseActive)
                        } else if (containsMouse && editorStoreModel.toolMode === "erase") {
                            appController.setBoardCursorActive(false)
                            appController.setEraseCursorActive(true, editorStoreModel.eraseRadiusPx, editorStoreModel.zoom)
                        }
                    }
                    onReleased: (mouse) => {
                        console.log("[qml] release", mouse.x, mouse.y, "button=", mouse.button)
                        boardPointerPressed = false
                        if (boardPanMode) {
                            boardPanMode = false
                            appController.setEraseCursorActive(root.eraseBrushAppCursor, editorStoreModel.eraseRadiusPx, editorStoreModel.zoom)
                            return
                        }
                        boardCanvas.pointerUp(mouse.x, mouse.y, mouse.button)
                        if (!containsMouse) {
                            appController.setBoardCursorActive(false)
                            appController.setEraseCursorActive(false, editorStoreModel.eraseRadiusPx, editorStoreModel.zoom)
                        } else if (editorStoreModel.toolMode === "draw") {
                            appController.setEraseCursorActive(editorStoreModel.drawStrokeEraseActive, editorStoreModel.eraseRadiusPx, editorStoreModel.zoom)
                            appController.setBoardCursorActive(!editorStoreModel.drawStrokeEraseActive)
                        } else if (editorStoreModel.toolMode === "erase") {
                            appController.setBoardCursorActive(false)
                            appController.setEraseCursorActive(true, editorStoreModel.eraseRadiusPx, editorStoreModel.zoom)
                        } else {
                            appController.setBoardCursorActive(false)
                            appController.setEraseCursorActive(false, editorStoreModel.eraseRadiusPx, editorStoreModel.zoom)
                        }
                    }
                    onDoubleClicked: (mouse) => {
                        const consumed = boardCanvas.pointerDoubleClick(mouse.x, mouse.y, mouse.button)
                        if (!consumed && mouse.button === Qt.LeftButton && !boardCanvas.hasSelectionAt(mouse.x, mouse.y)) {
                            boardPanMode = true
                            panLastX = mouse.x
                            panLastY = mouse.y
                            boardPointerPressed = false
                            appController.setBoardCursorActive(false)
                            appController.setEraseCursorActive(false, editorStoreModel.eraseRadiusPx, editorStoreModel.zoom)
                        }
                    }
                    onCanceled: {
                        boardPointerPressed = false
                        boardPanMode = false
                        appController.setBoardCursorActive(false)
                        appController.setEraseCursorActive(false, editorStoreModel.eraseRadiusPx, editorStoreModel.zoom)
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

    Dialog {
        id: assignDialog
        title: "Assign Character"
        modal: true
        standardButtons: Dialog.Ok | Dialog.Cancel
        width: 400
        contentItem: ColumnLayout {
            spacing: 8
            Label { text: "Enter exactly one ASCII character:" }
            TextField {
                id: assignInput
                placeholderText: "Example: A or #"
                selectByMouse: true
                Keys.onReturnPressed: assignDialog.accept()
                Keys.onEnterPressed: assignDialog.accept()
            }
            Label { text: "Join mode (export filename token):" }
            RowLayout {
                spacing: 12
                RadioButton {
                    text: "L"
                    checked: root.pendingAssignJoin === "L"
                    onCheckedChanged: if (checked) root.pendingAssignJoin = "L"
                }
                RadioButton {
                    text: "R"
                    checked: root.pendingAssignJoin === "R"
                    onCheckedChanged: if (checked) root.pendingAssignJoin = "R"
                }
                RadioButton {
                    text: "LR"
                    checked: root.pendingAssignJoin === "LR"
                    onCheckedChanged: if (checked) root.pendingAssignJoin = "LR"
                }
                RadioButton {
                    text: "N"
                    checked: root.pendingAssignJoin === "N"
                    onCheckedChanged: if (checked) root.pendingAssignJoin = "N"
                }
            }
        }
        onOpened: {
            assignInput.text = ""
            root.pendingAssignJoin = "N"
            const rows = editorStoreModel.selectionBoxesModel()
            for (let i = 0; i < rows.length; ++i) {
                const r = rows[i]
                if (r.id === root.pendingSelectionAssignId) {
                    const j = r.joinMode
                    if (j === "L" || j === "R" || j === "LR" || j === "N")
                        root.pendingAssignJoin = j
                    if (r.assigned && r.assignedAscii >= 0)
                        assignInput.text = String.fromCharCode(r.assignedAscii)
                    break
                }
            }
            Qt.callLater(() => assignInput.forceActiveFocus())
            Qt.callLater(() => assignInput.selectAll())
        }
        onAccepted: {
            appController.assignSelectionCharacter(root.pendingSelectionAssignId, assignInput.text, root.pendingAssignJoin)
        }
    }
}
