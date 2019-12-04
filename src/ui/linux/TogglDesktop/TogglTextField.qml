import QtQuick 2.12
import QtQuick.Controls 2.12

TextField {
    id: control
    color: mainPalette.text
    background: Rectangle {
        border.width: 1
        radius: 6
        border.color: mainPalette.borderColor
        color: "transparent"
    }
}
