import QtQuick 2.0

Rectangle {
    property Item control
    clip: true
    implicitWidth: 100
    implicitHeight: 32
    radius: 6

    property color borderColor: !control || control.enabled ? mainPalette.borderColor : disabledPalette.button
    //property color shadowColor: control.enabled ? control.checked ? "#595959" : control.hovered ? "#e5e5e5" : "#d4d4d4" : baseColor
    //property color baseColor: control.enabled ? control.checked ? "#7a7a7a" : control.hovered ? "#d4d4d4" : "#c3c3c3" : "#b2b2b2"
    property color shadowColor: !control || control.enabled ? !control || control.pressed || control.checked ? mixColors(mainPalette.button, mainPalette.buttonText, 0.5)
                                                                  : control.hovered ? mixColors(mainPalette.button, mainPalette.light, 0.5)
                                                                                    : mixColors(mainPalette.button, mainPalette.light, 0.5)
                                                : baseColor
    property color baseColor: !control || control.enabled ? !control || control.pressed || control.checked ? mixColors(mainPalette.buttonText, mainPalette.button, 0.66)
                                                                : control.hovered ? mixColors(mainPalette.button, mainPalette.light, 0.5)
                                                                                  : mainPalette.button
                                              : disabledPalette.button

    Behavior on shadowColor { ColorAnimation { duration: 120 } }
    Behavior on baseColor { ColorAnimation { duration: 120 } }

    color: mainPalette.base

    border.width: 1
    border.color: borderColor
}
