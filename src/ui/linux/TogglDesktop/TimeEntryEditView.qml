import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

Rectangle {
    anchors.fill: parent
    color: palette.window

    property var timeEntry: null


    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 3

        TextField {
            Layout.fillWidth: true
            text: timeEntry.Description
        }
        TextField {
            Layout.fillWidth: true
            text: timeEntry.ClientLabel + " . " + timeEntry.ProjectLabel
        }

        Button {
            Layout.alignment: Qt.AlignRight
            text: "Add project"
        }

        GridLayout {
            Layout.fillWidth: true
            columns: 4
            Text {
                text: "Duration:"
            }
            TextField {
                Layout.fillWidth: true
                Layout.columnSpan: 3
                text: timeEntry.Duration
            }

            Text {
                text: "Start-end time:"
            }
            TextField {
                Layout.fillWidth: true
                text: timeEntry.StartTimeString
            }
            Text {
                text: "-"
            }
            TextField {
                Layout.fillWidth: true
                text: timeEntry.EndTimeString
            }

            Text {
                text: "Date:"
            }
            TextField {
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
            Button {
                text: "Done"
                onClicked: toggl.viewTimeEntryList()
            }
            Button {
                text: "Delete"
                onClicked: toggl.deleteTimeEntry(timeEntry.GUID)
            }
            Button {
                text: "Cancel"
                onClicked: toggl.viewTimeEntryList()
            }
        }
        Text {
            text: timeEntry.lastUpdate()
        }
    }
}
