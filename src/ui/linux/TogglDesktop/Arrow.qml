import QtQuick 2.0

Item {
    width: 7
    height: 3
    Rectangle {
        smooth: true
        anchors.verticalCenter: parent.verticalCenter
        width: 4
        height: 1
        rotation: 45
        opacity: 0.5
    }
    Rectangle {
        smooth: true
        x: 3
        anchors.verticalCenter: parent.verticalCenter
        width: 4
        height: 1
        rotation: -45
        opacity: 0.5
    }
}
