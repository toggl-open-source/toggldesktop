import QtQuick 2.12
import QtQuick.Layouts 1.12

Item {
    z: index
    height: visible ? expanded ? itemHeight * 4 : itemHeight : 0
    Behavior on height { NumberAnimation { duration: 120 } }
    width: timeEntryList.viewportWidth

    property bool expanded: false
    onExpandedChanged: listView.itemExpanded = expanded
    property var listView
    property QtObject timeEntry: null

    Rectangle {
        z: 99999999
        anchors.fill: parent
        visible: opacity > 0.0
        opacity: listView.itemExpanded && !expanded ? 0.5 : 0.0
        Behavior on opacity { NumberAnimation { duration: 120 } }
        color: "dark gray"
        MouseArea { anchors.fill: parent }
    }

    Rectangle {
        anchors {
            fill: parent
            leftMargin: 12
            rightMargin: anchors.leftMargin
            topMargin: 0
        }
        color:  delegateMouse.containsMouse ? mainPalette.listBackground : timeEntry.GroupOpen ? mainPalette.listBackground : mainPalette.base

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
            hoverEnabled: !expanded
            anchors.fill: parent
            onClicked: {
                console.log("B")
                console.log("AAA " + index)
                expanded = !expanded
                listView.gotoIndex(index)
                /*
                if (timeEntry.Group)
                    toggl.toggleEntriesGroup(timeEntry.GroupName)
                else
                    toggl.editTimeEntry(timeEntry.GUID, "description")
                */
            }
        }
        RowLayout {
            anchors.fill: parent
            anchors.margins: 12
            spacing: 9

            Rectangle {
                Layout.alignment: Qt.AlignVCenter
                visible: (timeEntry.Group | timeEntry.GroupOpen) && ! expanded
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

            ColumnLayout {
                Layout.fillHeight: true
                Layout.fillWidth: true
                spacing: 3
                Text {
                    text: "Details"
                    visible: expanded
                    color: mainPalette.windowText
                    font.capitalization: Font.AllUppercase
                }
                TimeEntryLabel {
                    Layout.fillWidth: true

                    Behavior on width { NumberAnimation { duration: 120 } }

                    timeEntry: modelData
                    editable: expanded
                }

                TogglTextField {
                    Layout.fillWidth: true
                    opacity: expanded ? 1.0 : 0.0
                    Behavior on opacity { NumberAnimation { duration: 120 } }
                    visible: opacity > 0.0
                    placeholderText: qsTr("Add tags")
                }

                Text {
                    visible: expanded
                    text: qsTr("Duration")
                    color: mainPalette.windowText
                    font.capitalization: Font.AllUppercase
                }
                RowLayout {
                    visible: expanded
                    Layout.fillWidth: true
                    TextMetrics {
                        id: timeMetrics
                        text: "00:00:00 AM"
                    }

                    TogglTextField {
                        implicitWidth: timeMetrics.width
                        //Layout.minimumWidth: timeMetrics.width
                        text: timeEntry ? timeEntry.Duration : ""
                    }
                    Item {
                        height: 1
                        Layout.fillWidth: true
                    }

                    TogglTextField {
                        width: timeMetrics.width
                        implicitWidth: timeMetrics.width
                        //Layout.minimumWidth: timeMetrics.width
                        text: timeEntry ? timeEntry.StartTimeString : ""
                    }
                    Text {
                        text: "→"
                        color: mainPalette.text
                    }
                    TogglTextField {
                        width: timeMetrics.width
                        implicitWidth: timeMetrics.width
                        //Layout.minimumWidth: timeMetrics.width
                        text: timeEntry ? timeEntry.EndTimeString : ""
                    }
                }

                TogglTextField {
                    visible: expanded
                    Layout.fillWidth: true
                    Layout.columnSpan: 3
                    text: timeEntry ? (new Date(Date(timeEntry.Started)).toLocaleDateString(Qt.locale(), Locale.ShortFormat)) : ""
                }


                Text {
                    visible: expanded
                    Layout.fillWidth: true
                    Layout.columnSpan: 3
                    text: timeEntry ? timeEntry.WorkspaceName : ""
                    color: mainPalette.windowText
                }

                Item {
                    visible: expanded
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                }
            }

            Text {
                visible: !expanded
                Layout.alignment: Qt.AlignVCenter
                text: timeEntry.Duration
                color: mainPalette.text
            }
            Item {
                visible: !expanded
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
