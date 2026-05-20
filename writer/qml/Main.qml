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
    property string settingsSection: "view"

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

    Shortcut {
        sequence: StandardKey.Undo
        enabled: writerController.canUndo
        onActivated: writerController.undo()
    }
    Shortcut {
        sequence: StandardKey.Redo
        enabled: writerController.canRedo
        onActivated: writerController.redo()
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
                  : "No font folder selected. Choose “Select font folder” before generating G-codes."
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
        padding: 8
        contentItem: Flow {
            id: toolbarFlow
            width: parent.width
            spacing: 8
            flow: Flow.LeftToRight

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
            Button {
                text: "Undo"
                enabled: writerController.canUndo
                onClicked: writerController.undo()
            }
            Button {
                text: "Redo"
                enabled: writerController.canRedo
                onClicked: writerController.redo()
            }
            Button {
                id: generateGcodeBtn
                visible: writerController.viewMode === "handwriting"
                text: "Generate G-codes"
                onClicked: writerController.generateGcode()
                ToolTip.visible: hovered
                ToolTip.text: gcodeController.gcodeStale
                       ? "Content or plot settings changed — click to regenerate G-code"
                       : "Generate G-code from current handwriting layout"
                background: Rectangle {
                    radius: 4
                    color: gcodeController.gcodeStale ? "#eab308"
                           : (generateGcodeBtn.down ? "#d4d4d8"
                              : (generateGcodeBtn.hovered ? "#e4e4e7" : "#fafafa"))
                    border.color: gcodeController.gcodeStale ? "#ca8a04" : "#d4d4d8"
                    border.width: 1
                }
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
                implicitWidth: 280
                model: grblConnection.availablePorts
                enabled: grblConnection.serialAvailable && !grblConnection.connected
                         && !grblConnection.streaming

                background: Rectangle {
                    color: toolbarPortCombo.enabled ? "#3f3f46" : "#52525b"
                    border.color: "#71717a"
                    radius: 4
                }

                indicator: Text {
                    x: toolbarPortCombo.width - width - 10
                    y: toolbarPortCombo.topPadding + (toolbarPortCombo.availableHeight - height) / 2
                    text: "▼"
                    color: "#ffffff"
                    font.pixelSize: 9
                }

                delegate: ItemDelegate {
                    width: toolbarPortCombo.width
                    contentItem: Text {
                        text: {
                            const idx = grblConnection.availablePorts.indexOf(modelData)
                            return idx >= 0 && idx < grblConnection.portLabels.length
                                   ? grblConnection.portLabels[idx] : modelData
                        }
                        elide: Text.ElideRight
                        color: "#1e3a8a"
                    }
                    highlighted: toolbarPortCombo.highlightedIndex === index
                    background: Rectangle {
                        color: highlighted ? "#dbeafe" : "#ffffff"
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
                    color: "#ffffff"
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
                text: "Configure Run"
                enabled: !writerController.runActive
                onClicked: {
                    writerController.refreshPageMap()
                    configureRunPageSpin.value = writerController.runArmed
                            ? writerController.runStartPage : 0
                    configureRunDialog.open()
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
                implicitWidth: 84
                value: Math.round(writerController.settings.joinDistMm * 10)
                onValueModified: writerController.settings.joinDistMm = value / 10.0
            }
            Label {
                text: writerController.documentDirty ? "(unsaved changes)" : "Saved"
                color: writerController.documentDirty ? "#ca8a04" : "#3f3f46"
            }
            Label {
                width: Math.min(360, toolbarFlow.width > 0 ? toolbarFlow.width : 360)
                text: writerController.fontStatus
                wrapMode: Text.Wrap
                color: "#3f3f46"
                font.pixelSize: 11
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
                        text: "Open…"
                        implicitHeight: 28
                        onClicked: gcodeController.openGcodeFile()
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
                }

                ScrollView {
                    id: gcodeScroll
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.minimumHeight: 80
                    clip: true
                    ScrollBar.horizontal: ScrollBar { policy: ScrollBar.AsNeeded }
                    ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }

                    TextArea {
                        id: gcodeView
                        width: gcodeScroll.availableWidth
                        implicitHeight: Math.max(contentHeight, gcodeScroll.availableHeight)
                        wrapMode: TextArea.NoWrap
                        readOnly: false
                        cursorVisible: true
                        selectByMouse: true
                        selectByKeyboard: true
                        focusPolicy: Qt.StrongFocus
                        font.family: "monospace"
                        font.pixelSize: 11
                        color: "#1e3a8a"
                        selectedTextColor: "#ffffff"
                        selectionColor: "#2563eb"
                        placeholderText: "Generated G-code (editable). Lines starting with ; are ignored when sending."
                        placeholderTextColor: "#64748b"
                        property bool gcodeSync: false

                        Component.onCompleted: {
                            gcodeSync = true
                            text = gcodeController.generatedGcode
                            gcodeSync = false
                        }
                        onTextChanged: {
                            if (gcodeSync) return
                            gcodeSync = true
                            gcodeController.generatedGcode = text
                            gcodeSync = false
                        }
                        Connections {
                            target: gcodeController
                            function onGeneratedGcodeChanged() {
                                if (gcodeView.activeFocus) return
                                if (gcodeView.text === gcodeController.generatedGcode) return
                                gcodeView.gcodeSync = true
                                gcodeView.text = gcodeController.generatedGcode
                                gcodeView.gcodeSync = false
                            }
                        }
                        TapHandler {
                            onTapped: gcodeView.forceActiveFocus()
                        }
                        background: Rectangle {
                            color: "#ffffff"
                            border.color: gcodeView.activeFocus ? "#2563eb" : "#e4e4e7"
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
                    text: "Position (mm)"
                    font.bold: true
                    color: "#3f3f46"
                }

                Rectangle {
                    Layout.fillWidth: true
                    radius: 4
                    color: "#ffffff"
                    border.color: "#e4e4e7"
                    implicitHeight: positionCol.implicitHeight + 16

                    ColumnLayout {
                        id: positionCol
                        anchors.fill: parent
                        anchors.margins: 8
                        spacing: 6

                        GridLayout {
                            Layout.fillWidth: true
                            columns: 2
                            columnSpacing: 8
                            rowSpacing: 4

                            Label {
                                text: "X:"
                                font.bold: true
                                color: "#3f3f46"
                            }
                            Label {
                                text: grblConnection.positionKnown
                                      ? grblConnection.posX.toFixed(3) : "—"
                                font.family: "monospace"
                                font.pixelSize: 13
                                color: "#18181b"
                            }
                            Label {
                                text: "Y:"
                                font.bold: true
                                color: "#3f3f46"
                            }
                            Label {
                                text: grblConnection.positionKnown
                                      ? grblConnection.posY.toFixed(3) : "—"
                                font.family: "monospace"
                                font.pixelSize: 13
                                color: "#18181b"
                            }
                            Label {
                                text: "Z:"
                                font.bold: true
                                color: "#3f3f46"
                            }
                            Label {
                                text: grblConnection.positionKnown
                                      ? grblConnection.posZ.toFixed(3) : "—"
                                font.family: "monospace"
                                font.pixelSize: 13
                                color: "#18181b"
                            }
                        }

                        Button {
                            Layout.fillWidth: true
                            text: "Home"
                            implicitHeight: 32
                            enabled: grblConnection.connected && !grblConnection.streaming
                            ToolTip.visible: hovered
                            ToolTip.text: "Set current position as origin (G92 X0 Y0 Z30)"
                            onClicked: grblConnection.setWorkOriginHere()
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

                Label {
                    visible: grblConnection.connected
                             && grblConnection.machineState !== "Idle"
                             && grblConnection.machineState !== "Run"
                    text: "GRBL state: " + grblConnection.machineState
                    color: grblConnection.machineState === "Alarm" ? "#dc2626" : "#ca8a04"
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
                        enabled: writerController.runActive || grblConnection.streaming
                        onClicked: writerController.stopRun()
                    }
                    Button {
                        text: "Halt"
                        implicitHeight: 24
                        enabled: grblConnection.connected && (writerController.runActive || grblConnection.streaming)
                        onClicked: {
                            grblConnection.sendRealtimeCommand("!")
                            writerController.stopRun()
                        }
                    }
                    Item { Layout.fillWidth: true }
                }

                ScrollView {
                    id: consoleLogScroll
                    Layout.fillWidth: true
                    Layout.preferredHeight: 120
                    Layout.bottomMargin: 2
                    clip: true
                    ScrollBar.vertical: ScrollBar {
                        policy: ScrollBar.AsNeeded
                    }

                    TextArea {
                        id: consoleLog
                        width: consoleLogScroll.availableWidth
                        implicitHeight: contentHeight + topPadding + bottomPadding
                        readOnly: true
                        wrapMode: TextArea.Wrap
                        selectByMouse: true
                        topPadding: 6
                        bottomPadding: 10
                        font.family: "monospace"
                        font.pixelSize: 10
                        text: grblConnection.consoleLog
                        onTextChanged: consoleLog.scrollToEnd()
                        function scrollToEnd() {
                            cursorPosition = length
                            Qt.callLater(function () {
                                ensureVisible(length, 1)
                            })
                        }
                        Connections {
                            target: grblConnection
                            function onConsoleLogChanged() {
                                consoleLog.scrollToEnd()
                            }
                        }
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
                        placeholderText: grblConnection.connected
                                     ? "G-code or ! ~ ? (↑↓ history)"
                                     : "Connect to send commands"
                        font.family: "monospace"
                        font.pixelSize: 11
                        cursorVisible: true
                        selectByMouse: true
                        focusPolicy: Qt.StrongFocus
                        enabled: grblConnection.connected && !grblConnection.streaming
                                 && !grblConnection.commandBlocked
                        Keys.onPressed: function(event) {
                            if (event.key === Qt.Key_Up) {
                                text = grblConnection.commandHistoryOlder(text)
                                cursorPosition = text.length
                                event.accepted = true
                            } else if (event.key === Qt.Key_Down) {
                                text = grblConnection.commandHistoryNewer()
                                cursorPosition = text.length
                                event.accepted = true
                            }
                        }
                        onTextEdited: grblConnection.resetCommandHistoryBrowse()
                        onAccepted: {
                            if (text.trim().length > 0) {
                                grblConnection.sendUserCommand(text)
                                text = ""
                            }
                        }
                        TapHandler {
                            onTapped: consoleInput.forceActiveFocus()
                        }
                        onEnabledChanged: {
                            if (enabled) Qt.callLater(function () { consoleInput.forceActiveFocus() })
                        }
                    }
                    Button {
                        text: "Send"
                        implicitHeight: 32
                        enabled: grblConnection.connected && !grblConnection.streaming
                                 && !grblConnection.commandBlocked
                        onClicked: {
                            if (consoleInput.text.trim().length > 0) {
                                grblConnection.sendUserCommand(consoleInput.text)
                                consoleInput.text = ""
                            }
                        }
                    }
                }
            }
        }
    }

    Dialog {
        id: configureRunDialog
        title: "Configure Run"
        modal: true
        anchors.centerIn: parent
        standardButtons: Dialog.Close

        ColumnLayout {
            spacing: 12
            width: Math.max(implicitWidth, 320)

            Label {
                text: "Set the starting page. Earlier pages appear as already written (red trail). Press Run to execute from this page."
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                color: "#3f3f46"
                font.pixelSize: 12
            }

            RowLayout {
                spacing: 8
                Label {
                    text: "Page number"
                    color: "#3f3f46"
                }
                SpinBox {
                    id: configureRunPageSpin
                    from: 0
                    to: Math.max(0, writerController.pageCount - 1)
                    value: 0
                    editable: true
                    wheelEnabled: true
                }
                Label {
                    text: writerController.pageCount > 0
                          ? ("of " + (writerController.pageCount - 1))
                          : "(no pages)"
                    color: "#71717a"
                    font.pixelSize: 11
                }
            }

            Button {
                text: "Advance"
                Layout.alignment: Qt.AlignRight
                enabled: writerController.pageCount > 0
                onClicked: {
                    writerController.advanceRunToPage(configureRunPageSpin.value)
                    configureRunDialog.close()
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

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8
                    Button {
                        text: "View"
                        highlighted: root.settingsSection === "view"
                        onClicked: root.settingsSection = "view"
                    }
                    Button {
                        text: "Plotter"
                        highlighted: root.settingsSection === "plotter"
                        onClicked: root.settingsSection = "plotter"
                    }
                    Item { Layout.fillWidth: true }
                }

                ColumnLayout {
                    visible: root.settingsSection === "view"
                    Layout.fillWidth: true
                    spacing: 10

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

                Label { text: "Screen preview scale"; font.bold: true; color: root.settingsTitleColor }
                Label {
                    text: "Display-only multiplier on top of the built-in preview fit (does not change G-code). Default 100% ≈ half of page-fill; increase if preview still looks too small."
                    wrapMode: Text.Wrap
                    Layout.fillWidth: true
                    color: "#3f3f46"
                    font.pixelSize: 11
                }
                SpinBox {
                    from: 25
                    to: 300
                    stepSize: 5
                    editable: true
                    wheelEnabled: true
                    value: Math.round(writerController.settings.previewDisplayScale * 100)
                    onValueModified: writerController.settings.previewDisplayScale = value / 100.0
                }

                } // View section

                ColumnLayout {
                    visible: root.settingsSection === "plotter"
                    Layout.fillWidth: true
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

                Label { text: "Backlash interpolation over machine Y"; font.bold: true; color: root.settingsTitleColor }
                Label {
                    text: "Near/far errors are interpolated by |Y| between start and end. SpinBox values use entered/10 mm."
                    wrapMode: Text.Wrap
                    Layout.fillWidth: true
                    color: "#3f3f46"
                    font.pixelSize: 11
                }
                Label { text: "Y start for far-backlash ramp (mm)"; color: root.settingsTitleColor }
                SpinBox {
                    from: 0
                    to: 10000
                    stepSize: 1
                    editable: true
                    wheelEnabled: true
                    value: Math.round(writerController.settings.backlashYStartMm * 10)
                    onValueModified: writerController.settings.backlashYStartMm = value / 10.0
                }
                Label { text: "Y end for far-backlash ramp (mm)"; color: root.settingsTitleColor }
                SpinBox {
                    from: 0
                    to: 10000
                    stepSize: 1
                    editable: true
                    wheelEnabled: true
                    value: Math.round(writerController.settings.backlashYEndMm * 10)
                    onValueModified: writerController.settings.backlashYEndMm = value / 10.0
                }
                Label { text: "X error near (mm, actual = entered/10)"; color: root.settingsTitleColor }
                SpinBox {
                    from: 0
                    to: 200
                    stepSize: 1
                    editable: true
                    wheelEnabled: true
                    value: Math.round(writerController.settings.xErrorNearMm * 10)
                    onValueModified: writerController.settings.xErrorNearMm = value / 10.0
                }
                Label { text: "X error far (mm, actual = entered/10)"; font.bold: true; color: root.settingsTitleColor }
                SpinBox {
                    from: 0
                    to: 200
                    stepSize: 1
                    editable: true
                    wheelEnabled: true
                    value: Math.round(writerController.settings.xErrorMm * 10)
                    onValueModified: writerController.settings.xErrorMm = value / 10.0
                }

                Label { text: "Y error near (mm, actual = entered/10)"; color: root.settingsTitleColor }
                SpinBox {
                    from: 0
                    to: 200
                    stepSize: 1
                    editable: true
                    wheelEnabled: true
                    value: Math.round(writerController.settings.yErrorNearMm * 10)
                    onValueModified: writerController.settings.yErrorNearMm = value / 10.0
                }
                Label { text: "Y error far (mm, actual = entered/10)"; font.bold: true; color: root.settingsTitleColor }
                SpinBox {
                    from: 0
                    to: 200
                    stepSize: 1
                    editable: true
                    wheelEnabled: true
                    value: Math.round(writerController.settings.yErrorMm * 10)
                    onValueModified: writerController.settings.yErrorMm = value / 10.0
                }

                Label { text: "Path simplification"; font.bold: true; color: root.settingsTitleColor }
                Label {
                    text: "Speeds up writing by reducing tiny/collinear points before G-code generation."
                    wrapMode: Text.Wrap
                    Layout.fillWidth: true
                    color: "#3f3f46"
                    font.pixelSize: 11
                }
                Label { text: "Simplify tolerance (mm, actual = entered/100)"; color: root.settingsTitleColor }
                SpinBox {
                    from: 0
                    to: 100
                    stepSize: 1
                    editable: true
                    wheelEnabled: true
                    value: Math.round(writerController.settings.simplifyToleranceMm * 100)
                    onValueModified: writerController.settings.simplifyToleranceMm = value / 100.0
                }
                Label { text: "Min segment length (mm, actual = entered/100)"; color: root.settingsTitleColor }
                SpinBox {
                    from: 0
                    to: 100
                    stepSize: 1
                    editable: true
                    wheelEnabled: true
                    value: Math.round(writerController.settings.minSegmentMm * 100)
                    onValueModified: writerController.settings.minSegmentMm = value / 100.0
                }
                Label { text: "Collinear tolerance (mm, actual = entered/100)"; color: root.settingsTitleColor }
                SpinBox {
                    from: 0
                    to: 100
                    stepSize: 1
                    editable: true
                    wheelEnabled: true
                    value: Math.round(writerController.settings.collinearToleranceMm * 100)
                    onValueModified: writerController.settings.collinearToleranceMm = value / 100.0
                }
                Label { text: "Streaming preset"; color: root.settingsTitleColor }
                ComboBox {
                    Layout.fillWidth: true
                    model: ["safe", "balanced", "fast"]
                    currentIndex: {
                        const p = writerController.settings.streamingPreset
                        const idx = model.indexOf(p)
                        return idx >= 0 ? idx : 1
                    }
                    onActivated: writerController.settings.streamingPreset = currentText
                }
                Label { text: "Arc fitting (G2/G3)"; color: root.settingsTitleColor }
                CheckBox {
                    text: "Enable arc fit (disabled automatically while backlash compensation is active)"
                    checked: writerController.settings.arcFitEnabled
                    onToggled: writerController.settings.arcFitEnabled = checked
                }
                Label { text: "Arc fit tolerance (mm, actual = entered/100)"; color: root.settingsTitleColor }
                SpinBox {
                    from: 0
                    to: 100
                    stepSize: 1
                    editable: true
                    wheelEnabled: true
                    value: Math.round(writerController.settings.arcFitToleranceMm * 100)
                    onValueModified: writerController.settings.arcFitToleranceMm = value / 100.0
                }
                Label { text: "Machine tuning protocol"; font.bold: true; color: root.settingsTitleColor }
                Label {
                    text: "Tune GRBL short-segment dynamics for handwriting. Start conservative, test, then increase acceleration gradually."
                    wrapMode: Text.Wrap
                    Layout.fillWidth: true
                    color: "#3f3f46"
                    font.pixelSize: 11
                }
                Label { text: "Junction deviation $11"; color: root.settingsTitleColor }
                SpinBox {
                    from: 0
                    to: 200
                    stepSize: 1
                    editable: true
                    wheelEnabled: true
                    value: Math.round(writerController.settings.grblJunctionDeviation * 1000)
                    onValueModified: writerController.settings.grblJunctionDeviation = value / 1000.0
                }
                Label { text: "Acceleration X $120 (mm/s^2)"; color: root.settingsTitleColor }
                SpinBox {
                    from: 0
                    to: 200000
                    stepSize: 1
                    editable: true
                    wheelEnabled: true
                    value: Math.round(writerController.settings.grblAccelX * 10)
                    onValueModified: writerController.settings.grblAccelX = value / 10.0
                }
                Label { text: "Acceleration Y $121 (mm/s^2)"; color: root.settingsTitleColor }
                SpinBox {
                    from: 0
                    to: 200000
                    stepSize: 1
                    editable: true
                    wheelEnabled: true
                    value: Math.round(writerController.settings.grblAccelY * 10)
                    onValueModified: writerController.settings.grblAccelY = value / 10.0
                }
                Button {
                    text: "Apply tuning to GRBL ($11/$120/$121)"
                    enabled: grblConnection.connected && !grblConnection.streaming
                    onClicked: grblConnection.applyMotionTuning(
                                   writerController.settings.grblJunctionDeviation,
                                   writerController.settings.grblAccelX,
                                   writerController.settings.grblAccelY)
                }
                Label { text: "Servo fast toggle mode"; font.bold: true; color: root.settingsTitleColor }
                Label {
                    text: "Uses M3 S commands for pen up/down (faster on servo-zlink firmware) instead of Z motion."
                    wrapMode: Text.Wrap
                    Layout.fillWidth: true
                    color: "#3f3f46"
                    font.pixelSize: 11
                }
                CheckBox {
                    text: "Enable servo snap mode"
                    checked: writerController.settings.servoSnapMode
                    onToggled: writerController.settings.servoSnapMode = checked
                }
                Label { text: "Servo up S value"; color: root.settingsTitleColor }
                SpinBox {
                    from: 0
                    to: 100000
                    stepSize: 1
                    editable: true
                    wheelEnabled: true
                    value: Math.round(writerController.settings.servoUpS)
                    onValueModified: writerController.settings.servoUpS = value
                }
                Label { text: "Servo down S value"; color: root.settingsTitleColor }
                SpinBox {
                    from: 0
                    to: 100000
                    stepSize: 1
                    editable: true
                    wheelEnabled: true
                    value: Math.round(writerController.settings.servoDownS)
                    onValueModified: writerController.settings.servoDownS = value
                }

                Label { text: "CNC / pen"; font.bold: true; color: root.settingsTitleColor }
                Label { text: "Pen up Z (mm)"; color: root.settingsTitleColor }
                SpinBox {
                    from: -1000
                    to: 1000
                    stepSize: 1
                    editable: true
                    wheelEnabled: true
                    value: Math.round(writerController.settings.penUpZ)
                    onValueModified: writerController.settings.penUpZ = value
                }
                Label { text: "Pen down Z (mm)"; color: root.settingsTitleColor }
                SpinBox {
                    from: -1000
                    to: 1000
                    stepSize: 1
                    editable: true
                    wheelEnabled: true
                    value: Math.round(writerController.settings.penDownZ)
                    onValueModified: writerController.settings.penDownZ = value
                }

                } // Plotter section

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
