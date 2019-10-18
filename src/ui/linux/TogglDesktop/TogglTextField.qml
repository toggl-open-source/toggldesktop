import QtQuick 2.12
import QtQuick.Controls 2.12

TextField {
    id: control
    color: palette.text
    background: Rectangle {
        border.width: 1
        radius: 2
        border.color: mixColors(palette.base, palette.text, 0.33)
        color: palette.base
    }
}
