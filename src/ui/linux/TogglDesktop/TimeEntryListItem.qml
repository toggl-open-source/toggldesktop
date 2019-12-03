import QtQuick 2.12
import QtQuick.Layouts 1.12

Item {
    z: index
    height: visible ? itemHeight : 0
    width: timeEntryList.viewportWidth
    Rectangle {
        anchors {
            fill: parent
            leftMargin: 12
            rightMargin: anchors.leftMargin
            topMargin: 0
        }
        color: delegateMouse.containsMouse ? palette.listBackground : modelData.GroupOpen ? palette.listBackground : palette.base

        TogglShadowBox {
            anchors.fill: parent
            shadowWidth: 9
            shadowColor: palette.itemShadow
            backgroundColor: palette.listBackground
            sides: TogglShadowBox.Side.Left | TogglShadowBox.Side.Right | TogglShadowBox.Side.Bottom
        }

        Rectangle {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 1
            color: palette.listBackground
        }

        MouseArea {
            id: delegateMouse
            hoverEnabled: true
            anchors.fill: parent
            onClicked: {
                if (modelData.Group)
                    toggl.toggleEntriesGroup(modelData.GroupName)
                else
                    toggl.editTimeEntry(modelData.GUID, "description")
            }
        }
        RowLayout {
            anchors.fill: parent
            anchors.margins: 12
            spacing: 9
            Rectangle {
                Layout.alignment: Qt.AlignVCenter
                visible: modelData.Group
                width: 24
                height: 24
                radius: 4
                color: modelData.GroupOpen ? "dark green" : palette.base
                border {
                    color: modelData.GroupOpen ? "transparent" : palette.alternateBase
                    width: 0.5
                }
                Text {
                    color: modelData.GroupOpen ? "light green" : palette.alternateBase
                    anchors.centerIn: parent
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    text: modelData.GroupItemCount
                }
            }
            ColumnLayout {
                Layout.fillHeight: true
                Layout.fillWidth: true
                Text {
                    Layout.fillWidth: true
                    text: modelData.Description.length > 0 ? modelData.Description : "+ Add description"
                    color: modelData.Description.length > 0 ? palette.text : disabledPalette.text
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    font.pixelSize: 12
                    verticalAlignment: Text.AlignVCenter
                }
                RowLayout {
                    Layout.fillWidth: true
                    Rectangle {
                        height: 3
                        width: height
                        radius: height / 2
                        visible: modelData.ClientLabel.length > 0
                        color: modelData.Color
                        Layout.alignment: Qt.AlignVCenter
                    }
                    Text {
                        visible: modelData.ClientLabel.length > 0
                        text: modelData.ClientLabel
                        color: modelData.Color.length > 0 ? modelData.Color : palette.text
                        font.pixelSize: 12
                    }
                    Text {
                        text: modelData.ProjectLabel.length > 0 ? modelData.ProjectLabel : "+ Add project"
                        color: modelData.ProjectLabel.length > 0 ? palette.text : disabledPalette.text
                        font.pixelSize: 12
                    }
                    Text {
                        visible: modelData.TaskLabel.length > 0
                        text: modelData.TaskLabel
                        color: palette.text
                        font.pixelSize: 12
                    }
                    Item {
                        Layout.fillWidth: true
                    }
                }
            }
            Text {
                Layout.alignment: Qt.AlignVCenter
                text: modelData.Duration
                color: palette.text
            }
            Item {
                id: startButton
                opacity: delegateMouse.containsMouse ? 1.0 : 0.0
                width: 20
                height: 20
                MouseArea {
                    anchors.fill: parent
                    onClicked: toggl.continueTimeEntry(modelData.GUID)
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
