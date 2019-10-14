import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

Rectangle {
    id: root
    color: "black"
    height: 64

    property bool running: false
    property var runningTimeEntry: null

    function start() {
        if (!running && description.text.length > 0) {
            var dur = duration.text === "00:00" ? "" : duration.text
            toggl.start(description.text, dur, 0, 0, "", false)
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

    RowLayout {
        id: timerContainer
        height: parent.height
        width: parent.width - startButton.width
        ColumnLayout {
            Layout.fillHeight: true
            Layout.fillWidth: true
            Text {
                visible: running
                Layout.fillWidth: true
                Layout.fillHeight: true
                verticalAlignment: Text.AlignVCenter
                text: runningTimeEntry ? runningTimeEntry.Description : ""
                color: "white"
            }
            TextField {
                id: description
                focus: true
                visible: !running
                Layout.fillWidth: true
                onAccepted: start()
            }
        }
        ColumnLayout {
            Layout.fillHeight: true
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
            TextField {
                id: duration
                visible: !running
                Layout.preferredWidth: 64
                text: "00:00"
                onAccepted: start()
                validator: RegExpValidator {
                    regExp: /[0-9][0-9]:[0-9][0-9]/
                }
            }
        }
    }
    Rectangle {
        id: startButton
        width: parent.height
        height: parent.height
        anchors.left: timerContainer.right
        anchors.leftMargin: 3
        color: running ? "red" : "green"
        Text {
            text: running ? "Stop" : "Start"
            anchors.centerIn: parent
        }
        MouseArea {
            anchors.fill: parent
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
