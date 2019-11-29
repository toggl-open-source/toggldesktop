import QtQuick 2.0

Rectangle {
    id: startButton
    property bool running

    signal clicked

    width: 32
    height: width
    radius: width / 2

    color: running ? "#e20000" : "#47bc00"

    Rectangle {
        anchors.centerIn: parent
        width: startButton.width / 3
        height: startButton.width / 3
        radius: 2
        visible: running
    }


    Item {
        anchors.centerIn: parent
        visible: !running

        Item {
            x: -4.5
            y: -1
            /* Could be worth replacing with an actual picture eventually... */
            Rectangle {
                y: -Math.sqrt(3)/6 * width + radius / 2
                width: startButton.width / 2.7
                height: radius
                rotation: 30
                radius: 2
            }
            Rectangle {
                y: Math.sqrt(3)/6 * width - radius / 2
                width: startButton.width / 2.7
                height: radius
                rotation: -30
                radius: 2
            }
            Rectangle {
                x: -Math.sqrt(3)/6 * width - radius / 2
                width: startButton.width / 2.7
                height: radius
                rotation: 90
                radius: 2
            }
            Rectangle {
                x: -Math.sqrt(3)/6 * width + 1.6
                width: startButton.width / 2.7 - 3
                height: 2
                rotation: 90
            }
            Rectangle {
                x: -Math.sqrt(3)/6 * width + 3.6
                width: startButton.width / 2.7 - 5
                height: 2
                rotation: 90
            }
            Rectangle {
                x: -Math.sqrt(3)/6 * width + 5.6
                width: startButton.width / 2.7 - 7
                height: 2
                rotation: 90
            }
            Rectangle {
                x: -Math.sqrt(3)/6 * width + 7.6
                width: startButton.width / 2.7 - 9
                height: 2
                rotation: 90
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            startButton.clicked()
        }
    }
}
