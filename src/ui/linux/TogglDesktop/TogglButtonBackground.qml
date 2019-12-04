import QtQuick 2.0

Rectangle {
    property Item control
    clip: true
    implicitWidth: 100
    implicitHeight: 40
    radius: 2

    property color borderColor: mixColors(palette.base, mainPalette.text, 0.33)
    //property color shadowColor: control.enabled ? control.checked ? "#595959" : control.hovered ? "#e5e5e5" : "#d4d4d4" : baseColor
    //property color baseColor: control.enabled ? control.checked ? "#7a7a7a" : control.hovered ? "#d4d4d4" : "#c3c3c3" : "#b2b2b2"
    property color shadowColor: control.enabled ? control.pressed | control.checked ? mixColors(palette.button, mainPalette.buttonText, 0.5)
                                                                  : control.hovered ? mixColors(palette.button, mainPalette.light, 0.5)
                                                                                    : mixColors(palette.button, mainPalette.light, 0.5)
                                                : baseColor
    property color baseColor: control.enabled ? control.pressed | control.checked ? mixColors(palette.buttonText, mainPalette.button, 0.66)
                                                                : control.hovered ? mixColors(palette.button, mainPalette.light, 0.5)
                                                                                  : mainPalette.button
                                              : disabledPalette.button

    Behavior on shadowColor { ColorAnimation { duration: 120 } }
    Behavior on baseColor { ColorAnimation { duration: 120 } }

    gradient: Gradient {
        GradientStop { position: 0.0; color: shadowColor }
        GradientStop { position: 0.12; color: baseColor }
    }

    border.width: 1
    border.color: borderColor
}
