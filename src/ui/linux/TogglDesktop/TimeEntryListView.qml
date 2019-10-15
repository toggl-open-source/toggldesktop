import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

Rectangle {
    anchors.fill: parent
    color: palette.window
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
                    y: 2
                    width: timeEntryList.viewportWidth
                    height: 30
                    textFormat: Text.RichText
                    // this is actually rendering the width underneath so we force a repaint on each width change
                    text: "<table width=100% cellpadding=6> " + section + "<tr><td colspan=2><font color=transparent>" + width + "</font></td></tr></table>"
                    color: palette.windowText
                }

                delegate: Item {
                    height: 74
                    width: timeEntryList.viewportWidth
                    Rectangle {
                        anchors.fill: parent
                        anchors.margins: 0.5
                        color: "white"
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                if (modelData.Group)
                                    toggl.toggleEntriesGroup(modelData.GroupName)
                                else
                                    toggl.editTimeEntry(modelData.GUID, "description")
                            }
                        }
                        RowLayout {
                            anchors.fill: parent
                            Item { width: 1 }
                            ColumnLayout {
                                Layout.fillHeight: true
                                Layout.fillWidth: true
                                Text {
                                    Layout.fillWidth: true
                                    text: modelData.Description.length > 0 ? modelData.Description : "(no description)"
                                    font.pixelSize: 12
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
                                contentItem: Text {
                                    anchors.centerIn: parent
                                    verticalAlignment: Text.AlignVCenter
                                    horizontalAlignment: Text.AlignHCenter
                                    text: modelData.GroupOpen ? "▵" : modelData.GroupItemCount
                                }
                                checked: modelData.GroupOpen
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
                            Text {
                                Layout.alignment: Qt.AlignVCenter
                                text: "00:00:00"
                            }
                            Item { width: 1 }
                        }
                    }
                }
            }
        }
    }
}
