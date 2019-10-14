import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

Item {
    anchors.fill: parent
    ColumnLayout {
        anchors.fill: parent
        Rectangle {
            Layout.fillWidth: true
            color: "black"
            height: 64
            ColumnLayout {
                id: timerContainer
                height: parent.height
                width: parent.width - startButton.width
                Text {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    verticalAlignment: Text.AlignVCenter
                    text: "Current TE"
                    color: "white"
                }
            }
            Rectangle {
                id: startButton
                width: parent.height
                height: parent.height
                anchors.left: timerContainer.right
                color: "green"
                Text {
                    text: "Start"
                    anchors.centerIn: parent
                }
            }
        }
        ListView {
            id: timeEntryList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: toggl.timeEntries
            delegate: Item {
                height: 48
                width: timeEntryList.width
                Rectangle {
                    anchors.fill: parent
                    anchors.margins: 2
                    radius: 2
                    color: "white"
                    RowLayout {
                        anchors.fill: parent
                        Text {
                            Layout.fillHeight: true
                            Layout.fillWidth: true
                            text: modelData.Description
                            verticalAlignment: Text.AlignVCenter
                        }
                        Button {
                            implicitWidth: implicitHeight
                            contentItem: Image {
                                source: "qrc:/images/continue.svg"
                            }
                        }
                    }
                }
            }
        }
    }
}
