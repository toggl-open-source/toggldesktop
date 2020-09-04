import QtQuick 2.12

Item {
    id: root
    z: -1

    property real shadowWidth
    property color shadowColor
    property color backgroundColor

    enum Side {
        None = 0,
        Left = 1,
        Right = 2,
        Top = 4,
        Bottom = 8
    }

    property int sides: TogglShadowBox.Side.None

    // corners
    Rectangle {
        visible: (sides & TogglShadowBox.Side.Left) && (sides & TogglShadowBox.Side.Top)
        anchors.right: parent.left
        anchors.bottom: parent.top
        anchors.margins: -shadowWidth
        width: 2 * shadowWidth
        height: 2 * shadowWidth
        radius: shadowWidth
        rotation: 45
        z: -1
        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop { position: 0.0; color: backgroundColor }
            GradientStop { position: 0.5; color: shadowColor }
        }
    }

    Rectangle {
        visible: (sides & TogglShadowBox.Side.Right) && (sides & TogglShadowBox.Side.Top)
        anchors.left: parent.right
        anchors.bottom: parent.top
        anchors.margins: -shadowWidth
        width: 2 * shadowWidth
        height: 2 * shadowWidth
        radius: shadowWidth
        rotation: -45
        z: -1
        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop { position: 0.5; color: shadowColor }
            GradientStop { position: 1.0; color: backgroundColor }
        }
    }

    Rectangle {
        visible: (sides & TogglShadowBox.Side.Right) && (sides & TogglShadowBox.Side.Bottom)
        anchors.left: parent.right
        anchors.top: parent.bottom
        anchors.margins: -shadowWidth
        width: 2 * shadowWidth
        height: 2 * shadowWidth
        radius: shadowWidth
        rotation: 45
        z: -1
        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop { position: 0.5; color: shadowColor }
            GradientStop { position: 1.0; color: backgroundColor }
        }
    }

    Rectangle {
        visible: (sides & TogglShadowBox.Side.Left) && (sides & TogglShadowBox.Side.Bottom)
        anchors.right: parent.left
        anchors.top: parent.bottom
        anchors.margins: -shadowWidth
        width: 2 * shadowWidth
        height: 2 * shadowWidth
        radius: shadowWidth
        rotation: -45
        z: -1
        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop { position: 0.5; color: shadowColor }
            GradientStop { position: 0.0; color: backgroundColor }
        }
    }

    // sides
    Rectangle {
        visible: sides & TogglShadowBox.Side.Left
        anchors.right: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        width: shadowWidth
        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop { position: 0.0; color: backgroundColor }
            GradientStop { position: 1.0; color: shadowColor }
        }
    }

    Rectangle {
        visible: sides & TogglShadowBox.Side.Right
        anchors.left: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        width: shadowWidth
        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop { position: 0.0; color: shadowColor }
            GradientStop { position: 1.0; color: backgroundColor }
        }
    }

    Rectangle {
        visible: sides & TogglShadowBox.Side.Bottom
        anchors.top: parent.bottom
        anchors.right: parent.right
        anchors.left: parent.left

        height: shadowWidth
        gradient: Gradient {
            GradientStop { position: 0.0; color: shadowColor }
            GradientStop { position: 1.0; color: backgroundColor }
        }
    }

    Rectangle {
        visible: sides & TogglShadowBox.Side.Top
        anchors.bottom: parent.top
        anchors.right: parent.right
        anchors.left: parent.left

        height: shadowWidth
        gradient: Gradient {
            GradientStop { position: 0.0; color: backgroundColor }
            GradientStop { position: 1.0; color: shadowColor }
        }
    }
}
