import QtQuick 2.0
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

Button {
    id: control
    property alias color: background.color
    property alias textColor: text.color
    property alias iconSource: icon.source
    background: TogglButtonBackground {
        id: background
        control: control
    }
    contentItem: Rectangle {
        anchors.fill: parent
        color: "blue"
        RowLayout {
            anchors.fill: parent
            spacing: icon.status == Image.Ready ? 3 : 0
            Image {
                id: icon
                Layout.alignment: Qt.AlignCenter
            }
            Text {
                id: text
                Layout.alignment: Qt.AlignCenter
                color: control.enabled ? control.pressed | control.checked ? mainPalette.text
                                                                           : mainPalette.text
                                       : disabledPalette.text
                text: control.text
            }
        }
    }
}
