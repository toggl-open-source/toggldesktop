import QtQuick 2.0

Item {
    id: root
    property real size: 7
    property color color: "black"
    width: size
    height: size
    Rectangle {
        color: root.color
        width: Math.sqrt(2 * Math.pow(size, 2))
        anchors.centerIn: parent
        height: 1
        rotation: 45
    }
    Rectangle {
        color: root.color
        width: Math.sqrt(2 * Math.pow(size, 2))
        anchors.centerIn: parent
        height: 1
        rotation: -45
    }
}
