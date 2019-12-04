import QtQuick 2.12
import QtQuick.Layouts 1.12

ColumnLayout {
    Layout.fillHeight: true
    Layout.fillWidth: true

    property QtObject timeEntry: null

    Text {
        visible: timeEntry
        Layout.fillWidth: true
        text: timeEntry && timeEntry.Description.length > 0 ? timeEntry.Description : "+ Add description"
        color: timeEntry && timeEntry.Description.length > 0 ? mainPalette.text : disabledPalette.text
        wrapMode: Text.WrapAtWordBoundaryOrAnywhere
        font.pixelSize: 12
        verticalAlignment: Text.AlignVCenter
    }
    RowLayout {
        Layout.fillWidth: true
        Rectangle {
            visible: timeEntry && timeEntry.ProjectLabel.length > 0
            height: 8
            width: height
            radius: height / 2
            color: timeEntry ? timeEntry.Color : mainPalette.text
            Layout.alignment: Qt.AlignVCenter
        }
        Text {
            visible: timeEntry
            text: timeEntry && timeEntry.ProjectLabel.length > 0 ? timeEntry.ProjectLabel : "+ Add project"
            color: timeEntry && timeEntry.ProjectLabel.length > 0 ? (timeEntry.Color.length ? timeEntry.Color : mainPalette.text) : disabledPalette.text
            font.pixelSize: 12
        }
        Text {
            visible: timeEntry && timeEntry.TaskLabel.length > 0
            text: timeEntry && timeEntry.TaskLabel.length ? "- " + timeEntry.TaskLabel : ""
            color: mainPalette.text
            font.pixelSize: 12
        }
        Text {
            visible: timeEntry && timeEntry.ClientLabel.length > 0
            text: timeEntry && timeEntry.ClientLabel ? "• " + timeEntry.ClientLabel : ""
            color: mainPalette.text
            font.pixelSize: 12
        }
        Item {
            Layout.fillWidth: true
        }
    }
}
