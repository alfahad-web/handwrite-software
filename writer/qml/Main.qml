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
    title: writerController.projectFilePath.length > 0
           ? ("CNC handwriting — " + writerController.projectFilePath)
           : "CNC handwriting preview"
    color: "#f4f4f5"

    property string pendingAfterDiscard: ""

    readonly property string writerProjectFileName: {
        const p = writerController.projectFilePath
        if (!p || p.length === 0) return ""
        const i = Math.max(p.lastIndexOf("/"), p.lastIndexOf("\\"))
        return i >= 0 ? p.substring(i + 1) : p
    }

    Component.onCompleted: {
        grblConnection.refreshPorts()
        Qt.callLater(function () {
            if (writerController.viewMode === "typing")
                typeArea.forceActiveFocus()
        })
    }

    readonly property color settingsTitleColor: "#1e3a8a"

    function toggleGrblConnection() {
        if (grblConnection.connected) {
            grblConnection.disconnectPort()
        } else {
            grblConnection.refreshPorts()
            if (toolbarPortCombo.currentIndex >= 0)
                grblConnection.portName = toolbarPortCombo.currentText
            else if (grblConnection.portName.length === 0 && grblConnection.availablePorts.length > 0)
                grblConnection.portName = grblConnection.availablePorts[0]
            grblConnection.connectPort()
        }
    }

    Connections {
        target: writerController
        function onLineHeightCollisionWarning() {
            lineHeightWarnDialog.open()
        }
        function onFontFolderMissing(path) {
            fontMissingDialog.missingPath = path
            fontMissingDialog.open()
        }
        function onProjectIoError(message) {
            projectErrorDialog.errorText = message
            projectErrorDialog.open()
        }
    }

    Dialog {
        id: fontMissingDialog
        property string missingPath: ""
        parent: Overlay.overlay
        anchors.centerIn: parent
        modal: true
        title: "Font folder not found"
        standardButtons: Dialog.Ok
        width: Math.min(520, root.width - 48)
        contentItem: Label {
            text: fontMissingDialog.missingPath.length > 0
                  ? ("Font folder not found at memorized location:\n" + fontMissingDialog.missingPath)
                  : "Font folder not found."
            wrapMode: Text.Wrap
            color: "#18181b"
            width: parent ? parent.width : implicitWidth
        }
    }

    Dialog {
        id: projectErrorDialog
        property string errorText: ""
        parent: Overlay.overlay
        anchors.centerIn: parent
        modal: true
        title: "File error"
        standardButtons: Dialog.Ok
        width: Math.min(480, root.width - 48)
        contentItem: Label {
            text: projectErrorDialog.errorText
            wrapMode: Text.Wrap
            color: "#dc2626"
            width: parent ? parent.width : implicitWidth
        }
    }

    Dialog {
        id: unsavedDialog
        parent: Overlay.overlay
        anchors.centerIn: parent
        modal: true
        title: "Unsaved changes"
        width: Math.min(420, root.width - 48)
        standardButtons: Dialog.NoButton
        contentItem: ColumnLayout {
            spacing: 12
            width: Math.min(360, root.width - 64)
            Label {
                text: "Save changes before continuing?"
                wrapMode: Text.Wrap
                Layout.fillWidth: true
            }
            RowLayout {
                Layout.fillWidth: true
                spacing: 8
                Button {
                    text: "Save"
                    onClicked: {
                        writerController.saveWriterProject()
                        if (!writerController.documentDirty) {
                            unsavedDialog.close()
                            if (root.pendingAfterDiscard === "new")
                                writerController.newWriterProject()
                            else if (root.pendingAfterDiscard === "open")
                                writerController.openWriterProject()
                            root.pendingAfterDiscard = ""
                        }
                    }
                }
                Button {
                    text: "Discard"
                    onClicked: {
                        unsavedDialog.close()
                        if (root.pendingAfterDiscard === "new")
                            writerController.newWriterProject()
                        else if (root.pendingAfterDiscard === "open")
                            writerController.openWriterProject()
                        root.pendingAfterDiscard = ""
                    }
                }
                Button {
                    text: "Cancel"
                    onClicked: {
                        root.pendingAfterDiscard = ""
                        unsavedDialog.close()
                    }
                }
            }
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

    Popup {
        id: runCalcPopup
        parent: Overlay.overlay
        anchors.centerIn: parent
        modal: true
        closePolicy: Popup.NoAutoClose
        width: 300
        height: 140
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            BusyIndicator {
                Layout.alignment: Qt.AlignHCenter
                running: true
            }
            Label {
                text: "Doing calculations..."
                Layout.alignment: Qt.AlignHCenter
                color: "#18181b"
            }
        }
        background: Rectangle {
            color: "#ffffff"
            radius: 8
            border.color: "#d4d4d8"
        }
    }

    Connections {
        target: handCanvas
        function onRunPreparationStarted() {
            runCalcPopup.open()
        }
        function onRunPreparationFinished() {
            runCalcPopup.close()
        }
    }

    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            spacing: 8
            Button {
                id: fileButton
                text: root.writerProjectFileName.length > 0 ? root.writerProjectFileName : "File"
                onClicked: fileMenu.open()
            }
            Menu {
                id: fileMenu
                y: fileButton.height
                MenuItem {
                    text: "New"
                    onTriggered: {
                        if (writerController.documentDirty) {
                            root.pendingAfterDiscard = "new"
                            unsavedDialog.open()
                        } else {
                            writerController.newWriterProject()
                        }
                    }
                }
                MenuItem {
                    text: "Open"
                    onTriggered: {
                        if (writerController.documentDirty) {
                            root.pendingAfterDiscard = "open"
                            unsavedDialog.open()
                        } else {
                            writerController.openWriterProject()
                        }
                    }
                }
                MenuItem {
                    text: "Save"
                    onTriggered: writerController.saveWriterProject()
                }
            }
            ToolSeparator {}
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
            ToolSeparator {}
            ComboBox {
                id: toolbarPortCombo
                Layout.minimumWidth: 200
                Layout.preferredWidth: 300
                model: grblConnection.availablePorts
                enabled: grblConnection.serialAvailable && !grblConnection.connected
                         && !grblConnection.streaming

                delegate: ItemDelegate {
                    width: toolbarPortCombo.width
                    contentItem: Text {
                        text: {
                            const idx = grblConnection.availablePorts.indexOf(modelData)
                            return idx >= 0 && idx < grblConnection.portLabels.length
                                   ? grblConnection.portLabels[idx] : modelData
                        }
                        elide: Text.ElideRight
                        color: "#18181b"
                    }
                }

                contentItem: Text {
                    leftPadding: 12
                    rightPadding: toolbarPortCombo.indicator.width + toolbarPortCombo.spacing + 8
                    text: {
                        const idx = grblConnection.availablePorts.indexOf(grblConnection.portName)
                        if (idx >= 0 && idx < grblConnection.portLabels.length)
                            return grblConnection.portLabels[idx]
                        if (grblConnection.portName.length > 0)
                            return grblConnection.portName
                        return grblConnection.availablePorts.length > 0
                               ? "Select port…" : "No ports found"
                    }
                    color: "#18181b"
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }

                onActivated: grblConnection.portName = currentText

                Connections {
                    target: grblConnection
                    function onPortNameChanged() {
                        const idx = grblConnection.availablePorts.indexOf(grblConnection.portName)
                        if (idx >= 0) toolbarPortCombo.currentIndex = idx
                    }
                    function onAvailablePortsChanged() {
                        const idx = grblConnection.availablePorts.indexOf(grblConnection.portName)
                        toolbarPortCombo.currentIndex = idx >= 0 ? idx : 0
                    }
                }
            }
            Button {
                text: "↻"
                implicitWidth: 32
                ToolTip.visible: hovered
                ToolTip.text: "Refresh serial ports"
                enabled: grblConnection.serialAvailable && !grblConnection.streaming
                onClicked: grblConnection.refreshPorts()
            }
            Button {
                id: cncConnectBtn
                implicitHeight: 32
                implicitWidth: Math.max(96, cncConnectLabel.implicitWidth + 24)
                enabled: grblConnection.serialAvailable && !grblConnection.streaming
                onClicked: root.toggleGrblConnection()

                contentItem: Label {
                    id: cncConnectLabel
                    anchors.centerIn: parent
                    text: grblConnection.connected ? "Connected" : "Connect"
                    color: "#ffffff"
                    font.bold: true
                    font.pixelSize: 13
                }

                background: Rectangle {
                    radius: 6
                    color: grblConnection.connected ? "#16a34a" : "#dc2626"
                    border.color: grblConnection.connected ? "#15803d" : "#b91c1c"
                    border.width: 1
                    opacity: cncConnectBtn.down ? 0.85 : (cncConnectBtn.enabled ? 1.0 : 0.5)
                }
            }
            Button {
                text: !writerController.runActive ? "Run"
                      : (writerController.runPaused ? "Resume" : "Pause")
                onClicked: {
                    if (!writerController.runActive)
                        writerController.startRun()
                    else if (writerController.runPaused)
                        writerController.resumeRun()
                    else
                        writerController.pauseRun()
                }
            }
            Label {
                text: "Join (mm)"
                color: "#3f3f46"
            }
            SpinBox {
                from: 0
                to: 200
                stepSize: 1
                editable: true
                wheelEnabled: true
                value: Math.round(writerController.settings.joinDistMm * 10)
                onValueModified: writerController.settings.joinDistMm = value / 10.0
                Layout.preferredWidth: 84
            }
            Label {
                text: writerController.documentDirty ? "(unsaved changes)" : "Saved"
                color: writerController.documentDirty ? "#ca8a04" : "#3f3f46"
            }
            Item { Layout.fillWidth: true }
            Label {
                text: writerController.fontStatus
                elide: Text.ElideRight
                Layout.maximumWidth: 360
            }
        }
    }

    SplitView {
        id: mainSplit
        anchors.fill: parent
        orientation: Qt.Horizontal

        Item {
            SplitView.fillWidth: true
            SplitView.minimumWidth: 320
            clip: true

            Connections {
                target: writerController
                function onViewModeChanged() {
                    if (writerController.viewMode === "typing")
                        Qt.callLater(function () { typeArea.forceActiveFocus() })
                }
            }

            ScrollView {
                id: typeScroll
                anchors.fill: parent
                visible: writerController.viewMode === "typing"
                z: writerController.viewMode === "typing" ? 2 : 0
                clip: true
                ScrollBar.vertical: ScrollBar {
                    policy: ScrollBar.AlwaysOn
                }

                TextArea {
                    id: typeArea
                    width: typeScroll.availableWidth
                    implicitHeight: contentHeight + topPadding + bottomPadding
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
                ScrollBar.vertical: ScrollBar {
                    policy: ScrollBar.AlwaysOn
                }

                HandwritingCanvasItem {
                    id: handCanvas
                    width: handFlick.width
                    height: implicitHeight
                    implicitHeight: 400
                    controller: writerController
                }

                Binding {
                    target: handFlick
                    property: "interactive"
                    value: !handCanvas.glyphDragActive
                }
            }
        }

        Rectangle {
            id: gcodePanel
            SplitView.preferredWidth: 280
            SplitView.minimumWidth: 200
            SplitView.maximumWidth: Math.max(400, root.width * 0.55)
            color: "#fafafa"
            border.color: "#d4d4d8"

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 6
                spacing: 4

                Label {
                    text: "Generated G-code"
                    font.bold: true
                    color: "#3f3f46"
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 4
                    Button {
                        text: "Copy"
                        implicitHeight: 28
                        onClicked: gcodeController.copyToClipboard()
                    }
                    Button {
                        text: "Save…"
                        implicitHeight: 28
                        onClicked: gcodeController.saveGcodeFile()
                    }
                    Button {
                        text: "Send"
                        implicitHeight: 28
                        enabled: grblConnection.connected && !grblConnection.streaming
                        onClicked: grblConnection.streamProgram(gcodeController.generatedGcode)
                    }
                    Item { Layout.fillWidth: true }
                    Label {
                        text: gcodeController.gcodeStale ? "(stale)" : ""
                        color: "#ca8a04"
                        font.pixelSize: 11
                    }
                }

                ScrollView {
                    Layout.fillWidth: true
                    Layout.preferredHeight: parent.height * 0.55
                    Layout.fillHeight: true
                    clip: true
                    TextArea {
                        id: gcodeView
                        readOnly: true
                        wrapMode: TextArea.NoWrap
                        font.family: "monospace"
                        font.pixelSize: 11
                        text: gcodeController.generatedGcode
                        background: Rectangle {
                            color: "#ffffff"
                            border.color: "#e4e4e7"
                            radius: 4
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: "#d4d4d8"
                }

                Label {
                    text: "GRBL console"
                    font.bold: true
                    color: "#3f3f46"
                }

                Label {
                    visible: grblConnection.streaming
                    text: "Streaming " + Math.round(grblConnection.streamProgress * 100) + "%"
                    color: "#2563eb"
                    font.pixelSize: 11
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 4
                    Button {
                        text: "Clear"
                        implicitHeight: 24
                        onClicked: grblConnection.clearLog()
                    }
                    Button {
                        text: "Cancel"
                        implicitHeight: 24
                        enabled: grblConnection.streaming
                        onClicked: grblConnection.cancelStream()
                    }
                    Item { Layout.fillWidth: true }
                }

                ScrollView {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 120
                    clip: true
                    TextArea {
                        id: consoleLog
                        readOnly: true
                        wrapMode: TextArea.Wrap
                        font.family: "monospace"
                        font.pixelSize: 10
                        text: grblConnection.consoleLog
                        onTextChanged: cursorPosition = length
                        background: Rectangle {
                            color: "#18181b"
                            radius: 4
                        }
                        color: "#e4e4e7"
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 4
                    TextField {
                        id: consoleInput
                        Layout.fillWidth: true
                        implicitHeight: 32
                        placeholderText: "G-code or ! ~ ?"
                        font.family: "monospace"
                        font.pixelSize: 11
                        enabled: grblConnection.connected && !grblConnection.streaming
                        onAccepted: {
                            if (text.trim().length > 0) {
                                grblConnection.sendLine(text)
                                text = ""
                            }
                        }
                    }
                    Button {
                        text: "Send"
                        implicitHeight: 32
                        enabled: grblConnection.connected && !grblConnection.streaming
                        onClicked: {
                            if (consoleInput.text.trim().length > 0) {
                                grblConnection.sendLine(consoleInput.text)
                                consoleInput.text = ""
                            }
                        }
                    }
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
                Label {
                    text: "Also sets G-code F in mm/min (×600). Current: "
                          + Math.round(writerController.settings.feedRateMmPerMin)
                    wrapMode: Text.Wrap
                    Layout.fillWidth: true
                    color: "#3f3f46"
                    font.pixelSize: 11
                }
                SpinBox {
                    from: 1
                    to: 50000
                    stepSize: 1
                    editable: true
                    wheelEnabled: true
                    value: Math.round(writerController.settings.feedRateCmPerS * 1000)
                    onValueModified: writerController.settings.feedRateCmPerS = value / 1000.0
                }

                // SpinBox value is integer mm (stored settings remain cm): ±1 == ±1 mm on the page.
                Label { text: "Page width (mm)"; font.bold: true; color: root.settingsTitleColor }
                SpinBox {
                    from: 10
                    to: 50000
                    stepSize: 1
                    editable: true
                    wheelEnabled: true
                    value: Math.round(writerController.settings.pageWidthCm * 10)
                    onValueModified: writerController.settings.pageWidthCm = value / 10.0
                }

                Label { text: "Page height (mm)"; font.bold: true; color: root.settingsTitleColor }
                SpinBox {
                    from: 10
                    to: 50000
                    stepSize: 1
                    editable: true
                    wheelEnabled: true
                    value: Math.round(writerController.settings.pageHeightCm * 10)
                    onValueModified: writerController.settings.pageHeightCm = value / 10.0
                }

                Label { text: "Left margin (mm)"; font.bold: true; color: root.settingsTitleColor }
                SpinBox {
                    from: 0
                    to: 5000
                    stepSize: 1
                    editable: true
                    wheelEnabled: true
                    value: Math.round(writerController.settings.leftMarginCm * 10)
                    onValueModified: writerController.settings.leftMarginCm = value / 10.0
                }

                Label { text: "Right margin (mm)"; font.bold: true; color: root.settingsTitleColor }
                SpinBox {
                    from: 0
                    to: 5000
                    stepSize: 1
                    editable: true
                    wheelEnabled: true
                    value: Math.round(writerController.settings.rightMarginCm * 10)
                    onValueModified: writerController.settings.rightMarginCm = value / 10.0
                }

                Label { text: "Vertical gap (mm)"; font.bold: true; color: root.settingsTitleColor }
                SpinBox {
                    from: 0
                    to: 5000
                    stepSize: 1
                    editable: true
                    wheelEnabled: true
                    value: Math.round(writerController.settings.verticalGapCm * 10)
                    onValueModified: writerController.settings.verticalGapCm = value / 10.0
                }

                Label { text: "hx (mm)"; font.bold: true; color: root.settingsTitleColor }
                SpinBox {
                    from: 0
                    to: 5000
                    stepSize: 1
                    editable: true
                    wheelEnabled: true
                    value: Math.round(writerController.settings.hxCm * 10)
                    onValueModified: writerController.settings.hxCm = value / 10.0
                }

                Label { text: "hy (mm)"; font.bold: true; color: root.settingsTitleColor }
                SpinBox {
                    from: 0
                    to: 5000
                    stepSize: 1
                    editable: true
                    wheelEnabled: true
                    value: Math.round(writerController.settings.hyCm * 10)
                    onValueModified: writerController.settings.hyCm = value / 10.0
                }

                Label { text: "Line height (mm)"; font.bold: true; color: root.settingsTitleColor }
                SpinBox {
                    from: 1
                    to: 5000
                    stepSize: 1
                    editable: true
                    wheelEnabled: true
                    value: Math.round(writerController.settings.lineHeightCm * 10)
                    onValueModified: writerController.settings.lineHeightCm = value / 10.0
                }

                Label { text: "Join distance (mm)"; font.bold: true; color: root.settingsTitleColor }
                SpinBox {
                    from: 0
                    to: 200
                    stepSize: 1
                    editable: true
                    wheelEnabled: true
                    value: Math.round(writerController.settings.joinDistMm * 10)
                    onValueModified: writerController.settings.joinDistMm = value / 10.0
                }

                Label { text: "CNC / pen"; font.bold: true; color: root.settingsTitleColor }
                Label { text: "Pen up Z"; color: root.settingsTitleColor }
                SpinBox {
                    from: -1000
                    to: 1000
                    stepSize: 1
                    editable: true
                    wheelEnabled: true
                    value: Math.round(writerController.settings.penUpZ * 10)
                    onValueModified: writerController.settings.penUpZ = value / 10.0
                }
                Label { text: "Pen down Z"; color: root.settingsTitleColor }
                SpinBox {
                    from: -1000
                    to: 1000
                    stepSize: 1
                    editable: true
                    wheelEnabled: true
                    value: Math.round(writerController.settings.penDownZ * 10)
                    onValueModified: writerController.settings.penDownZ = value / 10.0
                }

                Label { text: "Font unit → mm scale"; font.bold: true; color: root.settingsTitleColor }
                Label {
                    text: "Multiply stroke coordinates from .txt to mm on paper (µm→mm: use 0.001). Each step ±1 here changes the scale by 0.00001 mm per font unit."
                    wrapMode: Text.Wrap
                    Layout.fillWidth: true
                    color: "#3f3f46"
                }
                // value/1e6 == fontUnitToCm; ±1 == ±1e-6 cm == ±10 nm per font unit on the scale factor (fontUnitToMm ±0.00001).
                SpinBox {
                    from: 1
                    to: 100000000
                    stepSize: 1
                    editable: true
                    wheelEnabled: true
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
