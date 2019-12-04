import QtQuick 2.12
import QtQuick.Layouts 1.12

Item {
    z: index
    height: visible ? itemHeight : 0
    width: timeEntryList.viewportWidth

    property QtObject timeEntry: null

    Rectangle {
        anchors {
            fill: parent
            leftMargin: 12
            rightMargin: anchors.leftMargin
            topMargin: 0
        }
        color: delegateMouse.containsMouse ? mainPalette.listBackground : timeEntry.GroupOpen ? mainPalette.listBackground : mainPalette.base

        TogglShadowBox {
            anchors.fill: parent
            shadowWidth: mainPalette.itemShadowSize
            shadowColor: mainPalette.itemShadow
            backgroundColor: mainPalette.listBackground
            sides: TogglShadowBox.Side.Left | TogglShadowBox.Side.Right | TogglShadowBox.Side.Bottom
        }

        Rectangle {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 1
            color: mainPalette.listBackground
        }

        MouseArea {
            id: delegateMouse
            hoverEnabled: true
            anchors.fill: parent
            onClicked: {
                if (timeEntry.Group)
                    toggl.toggleEntriesGroup(timeEntry.GroupName)
                else
                    toggl.editTimeEntry(timeEntry.GUID, "description")
            }
        }
        RowLayout {
            anchors.fill: parent
            anchors.margins: 12
            spacing: 9

            Rectangle {
                Layout.alignment: Qt.AlignVCenter
                visible: timeEntry.Group | timeEntry.GroupOpen
                opacity: timeEntry.GroupOpen ? 0.0 : 1.0
                width: 24
                height: 24
                radius: 4
                color: timeEntry.GroupOpen ? "dark green" : mainPalette.base
                border {
                    color: timeEntry.GroupOpen ? "transparent" : mainPalette.alternateBase
                    width: 0.5
                }
                Text {
                    color: timeEntry.GroupOpen ? "light green" : mainPalette.alternateBase
                    anchors.centerIn: parent
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    text: timeEntry.GroupItemCount
                }
            }

            TimeEntryLabel {
                Layout.fillHeight: true
                Layout.fillWidth: true

                timeEntry: modelData
            }

            Text {
                Layout.alignment: Qt.AlignVCenter
                text: timeEntry.Duration
                color: mainPalette.text
            }
            Item {
                id: startButton
                opacity: delegateMouse.containsMouse ? 1.0 : 0.0
                width: 20
                height: 20
                MouseArea {
                    anchors.fill: parent
                    onClicked: toggl.continueTimeEntry(timeEntry.GUID)
                }

                Item {
                    anchors.centerIn: parent
                    scale: 1.5

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
                            color: "#47bc00"
                        }
                        Rectangle {
                            y: Math.sqrt(3)/6 * width - radius / 2
                            width: startButton.width / 2.7
                            height: radius
                            rotation: -30
                            radius: 2
                            color: "#47bc00"
                        }
                        Rectangle {
                            x: -Math.sqrt(3)/6 * width - radius / 2
                            width: startButton.width / 2.7
                            height: radius
                            rotation: 90
                            radius: 2
                            color: "#47bc00"
                        }
                        Rectangle {
                            x: -Math.sqrt(3)/6 * width + 1.6
                            width: startButton.width / 2.7 - 3
                            height: 2
                            rotation: 90
                            color: "#47bc00"
                        }
                        Rectangle {
                            x: -Math.sqrt(3)/6 * width + 3.6
                            width: startButton.width / 2.7 - 5
                            height: 2
                            rotation: 90
                            color: "#47bc00"
                        }
                        Rectangle {
                            x: -Math.sqrt(3)/6 * width + 5.6
                            width: startButton.width / 2.7 - 7
                            height: 2
                            rotation: 90
                            color: "#47bc00"
                        }
                        Rectangle {
                            x: -Math.sqrt(3)/6 * width + 7.6
                            width: startButton.width / 2.7 - 9
                            height: 2
                            rotation: 90
                            color: "#47bc00"
                        }
                    }
                }
            }
        }
    }
}
