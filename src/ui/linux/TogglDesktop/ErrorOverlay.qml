import QtQuick 2.12
import QtQuick.Layouts 1.12

import QtQuick.Controls 2.12

Rectangle {
    id: errorOverlay
    color: "red"
    y: -height
    Behavior on y { NumberAnimation { duration: 120 } }
    Connections {
        target: toggl
        onDisplayError: {
            errorOverlay.y = 0
            errorText.text = errmsg
        }
    }
    RowLayout {
        anchors.fill: parent
        anchors.margins: 6
        Text {
            id: errorText
            Layout.fillHeight: true
            Layout.fillWidth: true
            verticalAlignment: Text.AlignVCenter
        }
        Button {
            text: "x"
            onClicked: errorOverlay.y = -errorOverlay.height
            implicitWidth: implicitHeight
        }
    }
}
