import QtQuick 2.12
import QtQuick.Controls 2.12

CheckBox {
    id: control
    indicator: Rectangle {
        implicitWidth: 18
        implicitHeight: 18
        x: control.leftPadding
        y: parent.height / 2 - height / 2
        radius: 3
        color: mainPalette.base
        border.color: mixColors(palette.base, mainPalette.text, 0.33)
        border.width: 1

        Rectangle {
            width: parent.width - 8
            height: parent.height - 8
            x: 4
            y: 4
            radius: 1
            antialiasing: true
            color: mainPalette.text
            opacity: control.checked ? 1.0 : 0.0
            Behavior on opacity { NumberAnimation { duration: 120 } }
            visible: opacity > 0.0
        }
    }

    contentItem: Text {
        text: control.text
        font: control.font
        opacity: enabled ? 1.0 : 0.3
        color: control.down ? "#17a81a" : "#21be2b"
        verticalAlignment: Text.AlignVCenter
        leftPadding: control.indicator.width + control.spacing
    }
}
