import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

Item {
    anchors.fill: parent
    ColumnLayout {
        anchors.fill: parent
        spacing: 0
        TimerView {
            Layout.fillWidth: true
            height: 64
        }

        ScrollView {
            id: timeEntryList
            Layout.fillWidth: true
            Layout.fillHeight: true
            property real viewportWidth: width - ScrollBar.vertical.width
            ListView {
                clip: true
                model: toggl.timeEntries
                section.property: "DateHeader"
                section.delegate: Text {
                    width: 100
                    height: 20
                    text: section
                    color: "white"
                }

                delegate: Item {
                    height: 48
                    width: timeEntryList.viewportWidth
                    Rectangle {
                        anchors.fill: parent
                        anchors.margins: 2
                        radius: 2
                        color: "white"
                        RowLayout {
                            anchors.fill: parent
                            ColumnLayout {
                                Layout.fillHeight: true
                                Layout.fillWidth: true
                                Text {
                                    Layout.fillWidth: true
                                    text: modelData.Description
                                    verticalAlignment: Text.AlignVCenter
                                }
                                RowLayout {
                                    Layout.fillWidth: true
                                    Text {
                                        text: modelData.ClientLabel
                                        font.pixelSize: 8
                                    }
                                    Text {
                                        text: modelData.ProjectLabel
                                        color: modelData.Color
                                        font.pixelSize: 8
                                    }
                                    Text {
                                        text: modelData.TaskLabel
                                        font.pixelSize: 8
                                    }
                                    Item {
                                        Layout.fillWidth: true
                                    }
                                }
                            }
                            Button {
                                visible: modelData.Group
                                implicitWidth: implicitHeight
                                text: modelData.GroupItemCount
                                onClicked: toggl.toggleEntriesGroup(modelData.GroupName)
                            }
                            Button {
                                implicitWidth: implicitHeight
                                contentItem: Text {
                                    anchors.centerIn: parent
                                    verticalAlignment: Text.AlignVCenter
                                    horizontalAlignment: Text.AlignHCenter
                                    text: "▸"
                                }
                                onClicked: toggl.continueTimeEntry(modelData.GUID)

                            }
                        }
                    }
                }
            }
        }
    }
}
