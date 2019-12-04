import QtQuick 2.0
import QtQuick.Controls 2.12

Button {
    id: control
    background: TogglButtonBackground {
        control: control
    }
    contentItem: Text {
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        color: control.enabled ? control.pressed | control.checked ? mainPalette.text
                                                                   : mainPalette.text
                               : disabledPalette.text
        text: control.text
    }
}
