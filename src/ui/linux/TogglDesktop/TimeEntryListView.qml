import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.0

Rectangle {
    id: root
    anchors.fill: parent
    color: backgroundColor

    property real sectionMargin: Math.max(2 * shadowWidth, 16)
    property real headerHeight: 32
    property real itemHeight: 74

    property real shadowWidth: palette.isDark ? 1 : 2
    property color shadowColor: palette.shadow
    property color backgroundColor: mixColors(palette.base, palette.alternateBase, 0.8)

    ColumnLayout {
        anchors.fill: parent
        spacing: 0
        TimerView {
            Layout.fillWidth: true
            height: 64
            z: 1
        }

        ScrollView {
            id: timeEntryList
            Layout.fillWidth: true
            Layout.fillHeight: true
            property real viewportWidth: width - ScrollBar.vertical.width
            ListView {
                clip: true
                model: toggl.timeEntries
                section.property: "modelData.DateHeader"
                section.delegate: Item {
                    height: headerHeight + sectionMargin
                    width: parent.width
                    Behavior on y {
                        NumberAnimation {
                            duration: 120
                        }
                    }

                    Rectangle {
                        y: sectionMargin
                        x: 12
                        width: timeEntryList.viewportWidth - 24
                        height: headerHeight
                        color: palette.base

                        Rectangle {
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
                            anchors.bottom: parent.top
                            anchors.right: parent.right
                            anchors.left: parent.left

                            height: shadowWidth
                            gradient: Gradient {
                                GradientStop { position: 0.0; color: backgroundColor }
                                GradientStop { position: 1.0; color: shadowColor }
                            }
                        }
                        Text {
                            anchors.fill: parent
                            anchors.leftMargin: 6
                            anchors.rightMargin: 6
                            textFormat: Text.RichText
                            // this is actually rendering the width underneath so we force a repaint on each width change
                            text: "<table width=100% cellpadding=6> " + section + "<tr><td colspan=2><font color=transparent>" + width + "</font></td></tr></table>"
                            color: palette.windowText
                        }
                    }
                }

                add: Transition {
                    id: addTransition
                    NumberAnimation {
                        properties: "y"
                        from: addTransition.ViewTransition.destination.y - itemHeight
                        duration: 120
                    }
                }
                addDisplaced: Transition {
                    NumberAnimation {
                        properties: "y"
                        duration: 120
                    }
                }

                moveDisplaced: Transition {
                    NumberAnimation {
                        from: -height
                        properties: "y"
                        duration: 120
                    }
                }

                move: Transition {
                    NumberAnimation {
                        properties: "y"
                        duration: 120
                    }
                }

                remove: Transition {
                    ParallelAnimation {
                        NumberAnimation {
                            duration: 120
                            properties: "opacity"
                            from: 1.0
                            to: 0.0
                        }
                        NumberAnimation {
                            duration: 120
                            properties: "x"
                            from: 0
                            to: width
                        }
                    }
                }
                removeDisplaced: Transition {
                    NumberAnimation {
                        properties: "y"
                        duration: 120
                    }
                }

                delegate: timeEntryDelegate
            }
        }
    }

    Component {
        id: timeEntryDelegate
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
                color: palette.base

                Rectangle {
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

                Rectangle {
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
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: 1
                    color: "#d4d4d4"
                }

                MouseArea {
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
                    TogglButton {
                        Layout.alignment: Qt.AlignVCenter
                        visible: modelData.Group
                        implicitWidth: implicitHeight
                        contentItem: Text {
                            anchors.centerIn: parent
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment: Text.AlignHCenter
                            text: modelData.GroupOpen ? "▵" : modelData.GroupItemCount
                        }
                        checked: modelData.GroupOpen
                        onClicked: toggl.toggleEntriesGroup(modelData.GroupName)
                    }
                    ColumnLayout {
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        Text {
                            Layout.fillWidth: true
                            text: modelData.Description.length > 0 ? modelData.Description : "(no description)"
                            color: palette.text
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
                                text: modelData.ClientLabel
                                color: modelData.Color.length > 0 ? modelData.Color : palette.text
                                font.pixelSize: 8
                            }
                            Text {
                                text: modelData.ProjectLabel
                                color: palette.text
                                font.pixelSize: 8
                            }
                            Text {
                                text: modelData.TaskLabel
                                color: palette.text
                                font.pixelSize: 8
                            }
                            Item {
                                Layout.fillWidth: true
                            }
                        }
                    }
                    TogglButton {
                        implicitWidth: implicitHeight
                        contentItem: Text {
                            anchors.centerIn: parent
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment: Text.AlignHCenter
                            text: "▸"
                        }
                        onClicked: toggl.continueTimeEntry(modelData.GUID)
                    }
                    Text {
                        Layout.alignment: Qt.AlignVCenter
                        text: modelData.Duration
                        color: palette.text
                    }
                }
            }
        }
    }
}
