import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

Rectangle {
    color: palette.alternateBase

    property var timeEntry: null


    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 9

        TogglTextField {
            Layout.fillWidth: true
            text: timeEntry.Description
        }
        TogglTextField {
            Layout.fillWidth: true
            text: timeEntry.ClientLabel + " . " + timeEntry.ProjectLabel
        }

        TogglButton {
            Layout.alignment: Qt.AlignRight
            text: "Add project"
        }

        GridLayout {
            Layout.fillWidth: true
            columns: 4
            Text {
                text: "Duration:"
            }
            TogglTextField {
                Layout.fillWidth: true
                Layout.columnSpan: 3
                text: timeEntry.Duration
            }

            Text {
                text: "Start-end time:"
            }
            TogglTextField {
                Layout.fillWidth: true
                text: timeEntry.StartTimeString
            }
            Text {
                text: "-"
            }
            TogglTextField {
                Layout.fillWidth: true
                text: timeEntry.EndTimeString
            }

            Text {
                text: "Date:"
            }
            TogglTextField {
                Layout.fillWidth: true
                Layout.columnSpan: 3
                text: new Date(Date(timeEntry.Started)).toLocaleDateString(Qt.locale(), Locale.ShortFormat)
            }

            Text {
                text: "Tags"
                Layout.fillHeight: true
            }
            ListView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.columnSpan: 3
                Frame {
                    anchors.fill: parent
                    background: Rectangle {
                        color: palette.base
                        border.color: palette.dark
                        border.width: 1
                    }
                    ColumnLayout {
                        anchors.fill: parent
                        Flow {
                            Layout.fillHeight: true
                            Layout.fillWidth: true
                            Repeater {
                                model: timeEntry.Tags
                                delegate: Item {
                                    width: childrenRect.width + 4
                                    height: childrenRect.height + 4
                                    Rectangle {
                                        x: 2
                                        y: 2
                                        width: selectedTagLayout.width + 4
                                        height: selectedTagLayout.height + 4
                                        color: "white"
                                        border.width: 1
                                        border.color: "#d4d4d4"
                                        radius: 2

                                        RowLayout {
                                            x: 2
                                            y: 2
                                            id: selectedTagLayout
                                            Text {
                                                id: selectedTagText
                                                text: modelData
                                                font.pixelSize: 10

                                                Rectangle {
                                                    Behavior on opacity { NumberAnimation { } }
                                                    opacity: selectedTagMouse.containsMouse ? 1.0 : 0.0
                                                    visible: opacity > 0.0
                                                    width: 10
                                                    height: 10
                                                    radius: 3
                                                    anchors.centerIn: parent
                                                    color: "red"
                                                    Text {
                                                        anchors.centerIn: parent
                                                        text: "x"
                                                        font.pointSize: 8
                                                    }
                                                }
                                            }
                                        }
                                        MouseArea {
                                            id: selectedTagMouse
                                            anchors.fill: parent
                                            hoverEnabled: true
                                            onClicked: {
                                                var list = timeEntry.Tags
                                                var index = list.indexOf(modelData);
                                                if (index > -1) {
                                                    list.splice(index, 1);
                                                    toggl.setTimeEntryTags(timeEntry.GUID, list.sort().join("\t"))
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            height: 1
                            color: "#c3c3c3"
                        }


                        Flow {
                            clip: true
                            Layout.fillHeight: true
                            Layout.fillWidth: true
                            Repeater {
                                model: toggl.tags.filter(n => !timeEntry.Tags.includes(n))
                                delegate: Item {
                                    width: childrenRect.width + 4
                                    height: childrenRect.height + 4
                                    Rectangle {
                                        x: 2
                                        y: 2
                                        width: tagLayout.width + 4
                                        height: tagLayout.height + 4
                                        color: "white"
                                        border.width: 1
                                        border.color: "#d4d4d4"
                                        radius: 2

                                        RowLayout {
                                            id: tagLayout
                                            x: 2
                                            y: 2
                                            Text {
                                                id: tagText
                                                text: modelData
                                                font.pixelSize: 10
                                                Rectangle {
                                                    Behavior on opacity { NumberAnimation { } }
                                                    opacity: tagMouse.containsMouse ? 1.0 : 0.0
                                                    visible: opacity > 0.0
                                                    width: 10
                                                    height: 10
                                                    radius: 3
                                                    anchors.centerIn: parent
                                                    color: "green"
                                                    Text {
                                                        anchors.centerIn: parent
                                                        text: "+"
                                                        font.pointSize: 8
                                                    }
                                                }
                                            }
                                        }
                                        MouseArea {
                                            id: tagMouse
                                            anchors.fill: parent
                                            hoverEnabled: true
                                            onClicked: {
                                                var list = timeEntry.Tags
                                                list.push(modelData)
                                                toggl.setTimeEntryTags(timeEntry.GUID, list.sort().join("\t"))
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            Text {
                                text: "New:"
                            }
                            TogglTextField {
                                Layout.fillWidth: true
                                id: newTagField
                                onAccepted: {
                                    if (text.length > 0) {
                                        var list = timeEntry.Tags
                                        list.push(newTagField.text)
                                        toggl.setTimeEntryTags(timeEntry.GUID, list.sort().join("\t"))
                                        newTagField.text = ""
                                    }
                                }
                            }
                            TogglButton {
                                Layout.preferredWidth: 64
                                text: "Add"
                                onClicked: {
                                    if (text.length > 0) {
                                        var list = timeEntry.Tags
                                        list.push(newTagField.text)
                                        toggl.setTimeEntryTags(timeEntry.GUID, list.sort().join("\t"))
                                        newTagField.text = ""
                                    }
                                }
                            }
                        }
                    }
                }
            }

            Text {
                text: "Workspace:"
            }
            Label {
                Layout.fillWidth: true
                Layout.columnSpan: 3
                text: timeEntry.WorkspaceName
            }
        }
        RowLayout {
            TogglButton {
                text: "Done"
                onClicked: toggl.viewTimeEntryList()
            }
            TogglButton {
                text: "Delete"
                onClicked: toggl.deleteTimeEntry(timeEntry.GUID)
            }
            TogglButton {
                text: "Cancel"
                onClicked: toggl.viewTimeEntryList()
            }
        }
        Text {
            text: timeEntry.lastUpdate()
        }
    }
}
