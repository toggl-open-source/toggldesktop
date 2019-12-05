import QtQuick 2.12
import QtQuick.Controls 2.12

TextField {
    id: control
    color: mainPalette.text
    property bool flat: false
    background: Rectangle {
        border.width: flat ? 0 : 1
        radius: 6
        border.color: mainPalette.borderColor
        color: "transparent"
    }
}
