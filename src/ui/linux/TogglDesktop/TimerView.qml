import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

Rectangle {
    id: root
    color: mixColors(palette.window, palette.base, 0.9)
    height: 64

    property bool running: false
    property var runningTimeEntry: null

    function start() {
        if (!running) {
            toggl.start(description.text, "", 0, 0, "", false)
            description.text = ""
        }
    }

    Connections {
        target: toggl
        onDisplayStoppedTimerState: {
            running = false
            runningTimeEntry = null
        }
        onDisplayRunningTimerState: {
            running = true
            runningTimeEntry = view
        }
    }

    MouseArea {
        anchors.fill: timerContainer
        onClicked: {
            if (running) {
                toggl.editRunningTimeEntry("description")
                mouse.accepted = true
            }
            else {
                mouse.accepted = false
            }
        }
    }

    RowLayout {
        id: timerContainer
        x: 6
        height: parent.height
        width: parent.width - 12
        ColumnLayout {
            Layout.fillHeight: true
            Layout.fillWidth: true
            spacing: 0
            Text {
                visible: running
                Layout.fillWidth: true
                Layout.fillHeight: true
                verticalAlignment: Text.AlignVCenter
                text: runningTimeEntry && runningTimeEntry.Description.length > 0 ? " " + runningTimeEntry.Description : " (no description)"
                color: "white"
                font.pixelSize: 12
            }
            TextField {
                id: description
                focus: true
                visible: !running
                Layout.fillWidth: true
                background: Item {}
                font.pixelSize: 12
                placeholderText: "What are you doing?"
                placeholderTextColor: "light gray"
                color: "white"
                Keys.onUpPressed: autocomplete.upPressed()
                Keys.onDownPressed: autocomplete.downPressed()
                Keys.onEscapePressed: autocomplete.visible = false
                Keys.onReturnPressed: {
                    if (autocomplete.visible && autocomplete.currentItem) {
                        console.log("something")
                        autocomplete.visible = false
                    }
                    else {
                        if (acceptableInput) {
                            start()
                        }
                    }
                }
                onTextEdited: {
                    if (focus) {
                        //autocomplete.visible = true
                    }
                }
                onFocusChanged: if (!focus) autocomplete.visible = false
                AutocompleteView {
                    id: autocomplete
                    visible: false
                    anchors{
                        top: parent.bottom
                        left: parent.left
                        right: parent.right
                    }
                    filter: description.text
                    //model: toggl.minitimerAutocomplete
                }
                Rectangle {
                    anchors.fill: parent
                    anchors.topMargin: -1
                    anchors.bottomMargin: -1
                    radius: height / 2
                    color: mixColors(palette.window, palette.alternateBase, 0.8)
                    z: -1
                }
            }
            RowLayout {
                visible: runningTimeEntry && runningTimeEntry.ProjectLabel.length > 0
                Button {
                    implicitWidth: 12
                    implicitHeight: 12
                    text: "x"
                }
                Text {
                    text: runningTimeEntry ? runningTimeEntry.ProjectLabel : ""
                    color: runningTimeEntry && runningTimeEntry.Color.length > 0 ? runningTimeEntry.Color : "white"
                    font.pixelSize: 8
                }
                Text {
                    visible: runningTimeEntry && runningTimeEntry.TaskLabel.length > 0
                    text: runningTimeEntry && runningTimeEntry.TaskLabel.length ? "• " + runningTimeEntry.TaskLabel : ""
                    color: "white"
                }
            }
            RowLayout {
                visible: runningTimeEntry && runningTimeEntry.ClientLabel.length > 0
                Button {
                    implicitWidth: 12
                    implicitHeight: 12
                    text: "x"
                }
                Text {
                    text: runningTimeEntry ? runningTimeEntry.ClientLabel : ""
                    color: "white"
                    font.pixelSize: 8
                }
            }
        }
        Text {
            visible: running
            Layout.fillHeight: true
            verticalAlignment: Text.AlignVCenter
            text: runningTimeEntry ? runningTimeEntry.Duration : ""
            Timer {
                running: root.running
                interval: 100
                repeat: true
                onTriggered: {
                    parent.text = toggl.formatDurationInSecondsHHMMSS(new Date().getTime() / 1000 - runningTimeEntry.Started)
                }
            }

            color: "white"
        }

        StartButton {
            id: startButton
            running: root.running
            Layout.alignment: Qt.AlignVCenter
            onClicked: {
                if (running) {
                    toggl.stop()
                }
                else {
                    start()
                }
            }
        }
    }
}
