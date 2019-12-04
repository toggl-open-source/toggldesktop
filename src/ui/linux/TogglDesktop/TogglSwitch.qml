import QtQuick 2.12
import QtQuick.Controls 2.12

Switch {
    id: control
    width: 40
    indicator: Rectangle {
        implicitWidth: 40
        implicitHeight: 20
        x: control.leftPadding
        y: parent.height / 2 - height / 2
        radius: 13
        color: mainPalette.base
        border.width: 1
        border.color: mixColors(palette.base, mainPalette.text, 0.33)

        Rectangle {
            x: control.checked ? parent.width - width - 1 : 1
            y: 1

            width: parent.height - 2
            height: parent.height - 2
            radius: parent.height / 2 - 1

            color: mainPalette.button

            Behavior on x {
                NumberAnimation {
                    duration: 120
                }
            }
            border.color: mixColors(palette.base, mainPalette.text, 0.33)
        }
    }
}
