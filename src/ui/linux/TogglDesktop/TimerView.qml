import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

Rectangle {
    color: "black"
    height: 64

    property bool running: false
    property var runningTimeEntry: null

    function start() {
        if (!running && description.text.length > 0) {
            toggl.start(description.text, "", 0, 0, "", false)
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
                visible: !running
                Layout.fillWidth: true
                onAccepted: start()
            }
        }
        Text {
            Layout.fillHeight: true
            verticalAlignment: Text.AlignVCenter
            text: running ? runningTimeEntry.Duration : "00:00"
            color: "white"
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
