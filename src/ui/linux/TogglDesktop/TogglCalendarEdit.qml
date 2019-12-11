import QtQuick 2.6
import Qt.labs.calendar 1.0
import QtQuick.Controls 1.4

TogglButtonBackground {
    id: calendarComponent
    visible: expanded
    width: parent.width
    clip: true
    Behavior on height { NumberAnimation { duration: 120 } }

    property bool calendarOpen: false
    height: calendar.visible ? implicitHeight + calendar.height + 12: implicitHeight

    property var date: timeEntry ? new Date(timeEntry.Started * 1000) : null
    property string dayOfWeek: date ? date.toLocaleDateString(Qt.locale(), "dddd") : 0
    property int day: date ? date.toLocaleDateString(Qt.locale(), "dd") : 0
    property int month: date ? date.toLocaleDateString(Qt.locale(), "MM") : 0
    property int year: date ? date.toLocaleDateString(Qt.locale(), "yyyy") : 0

    Item {
        id: calendar
        visible: calendarOpen
        y: parent.implicitHeight + 6
        width: parent.width
        height: childrenRect.height
        DayOfWeekRow {
            id: dayOfWeekRow
            width: monthScrollView.viewport.width
            delegate: Text {
                color: mainPalette.text
                text: model.shortName
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }

        ScrollView {
            id: monthScrollView
            anchors.top: dayOfWeekRow.bottom
            anchors.margins: 6
            width: parent.width
            height: thisMonth.height
            Component.onCompleted: {
                flickableItem.contentY = thisMonth.y
            }
            Item {
                x: 3
                y: 3
                width: monthScrollView.viewport.width - 6
                height: nextNextMonth.y + nextNextMonth.height - nextNextMonth.endOffset + 6
                TogglCalendarEditMonth {
                    id: previousPreviousMonth
                    width: parent.width
                    month: (calendarComponent.month - 3) % 12
                    year: calendarComponent.year + Math.floor((calendarComponent.month - 3) / 12)
                }
                TogglCalendarEditMonth {
                    id: previousMonth
                    y: previousPreviousMonth.y + previousPreviousMonth.height - previousPreviousMonth.offset
                    width: parent.width
                    month: (calendarComponent.month - 2) % 12
                    year: calendarComponent.year + Math.floor((calendarComponent.month - 2) / 12)
                }
                TogglCalendarEditMonth {
                    id: thisMonth
                    y: previousMonth.y + previousMonth.height - previousMonth.offset
                    width: parent.width
                    month: (calendarComponent.month - 1) % 12
                    year: calendarComponent.year + Math.floor((calendarComponent.month - 1) / 12)
                }
                TogglCalendarEditMonth {
                    id: nextMonth
                    y: thisMonth.y + thisMonth.height - thisMonth.offset
                    width: parent.width
                    month: (calendarComponent.month - 0) % 12
                    year: calendarComponent.year + Math.floor((calendarComponent.month - 0) / 12)
                }
                TogglCalendarEditMonth {
                    id: nextNextMonth
                    y: nextMonth.y + nextMonth.height - nextMonth.offset
                    width: parent.width
                    month: (calendarComponent.month + 1) % 12
                    year: calendarComponent.year + Math.floor((calendarComponent.month + 1) / 12)
                }
            }
        }
    }

    Rectangle {
        anchors {
            left: parent.left
            right: parent.right
            bottom: calendar.top
            bottomMargin: 6
        }
        visible: calendarOpen
        height: 1
        color: parent.borderColor
    }

    MouseArea {
        anchors {
            top: parent.top
            left: leftSeparator.right
            right: rightSeparator.left
        }
        height: parent.implicitHeight

        onClicked: {
            console.log("This would open a calendar")
            calendarOpen = !calendarOpen
        }
    }

    Rectangle {
        id: leftSeparator
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.leftMargin: 32
        height: parent.implicitHeight
        width: 1
        color: parent.borderColor
    }

    MouseArea {
        anchors {
            right: leftSeparator.left
            top: parent.top
            left: parent.left
        }
        height: parent.implicitHeight
        onClicked: console.log("Date--")
        Arrow {
            anchors.centerIn: parent
            rotation: 90
        }
    }
    Rectangle {
        id: rightSeparator
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.rightMargin: 32
        width: 1
        height: parent.implicitHeight
        color: parent.borderColor
    }
    MouseArea {
        anchors {
            left: rightSeparator.right
            top: parent.top
            right: parent.right
        }
        height: parent.implicitHeight
        onClicked: console.log("Date++")
        Arrow {
            anchors.centerIn: parent
            rotation: -90
        }
    }

    Text {
        color: mainPalette.text
        anchors.horizontalCenter: parent.horizontalCenter
        y: parent.implicitHeight / 2 - height / 2
        //text: timeEntry ? (new Date(Date(timeEntry.Started)).toLocaleDateString(Qt.locale(), Locale.ShortFormat)) : ""
        text: calendarComponent.dayOfWeek + ", " + calendarComponent.day + "/" + calendarComponent.month + "/" + calendarComponent.year
    }
}
