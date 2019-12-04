import QtQuick 2.12
import QtQuick.Controls 2.12

TextField {
    id: control
    color: mainPalette.text
    background: Rectangle {
        border.width: 1
        radius: 2
        border.color: mixColors(palette.base, mainPalette.text, 0.33)
        color: "transparent"
    }
}
