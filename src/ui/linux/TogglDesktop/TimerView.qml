import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

Rectangle {
    id: root
    color: palette.base
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
        x: 12
        height: parent.height
        width: parent.width - 24

        ColumnLayout {
            Layout.fillHeight: true
            Layout.fillWidth: true
            spacing: 6

            Item {
                Layout.fillHeight: true
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
                        autocomplete.visible = true
                    }
                }
                onFocusChanged: if (!focus) autocomplete.visible = false
                Rectangle {
                    anchors.fill: parent
                    anchors.topMargin: -1
                    anchors.bottomMargin: -1
                    radius: height / 2
                    color: mixColors(palette.window, palette.alternateBase, 0.8)
                    z: -1
                }
            }
            TimeEntryLabel {
                Layout.fillHeight: true
                Layout.fillWidth: true
                timeEntry: runningTimeEntry
                visible: runningTimeEntry
            }

            Item {
                Layout.fillHeight: true
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

    Rectangle {
        anchors {
            left: parent.left
            right: parent.right
            top: parent.bottom
        }
        height: 12

        gradient: Gradient {
            orientation: Gradient.Vertical
            GradientStop { position: 0.0; color: "#33888888" }
            GradientStop { position: 1.0; color: "#00888888" }
        }
    }

    AutocompleteView {
        id: autocomplete
        visible: false
        anchors{
            top: parent.bottom
            left: parent.left
            right: parent.right
        }
        maximumHeight: mainWindow.height - parent.height
        filter: description.text
        model: toggl.minitimerAutocomplete
    }
}
