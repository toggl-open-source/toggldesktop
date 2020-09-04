import QtQuick 2.6
import Qt.labs.calendar 1.0
import QtQuick.Controls 1.4

MonthGrid {
    id: monthGrid
    property Item firstNextMonth: null
    property real offset: firstNextMonth ? (height - firstNextMonth.y) : 0
    property Item firstWeekDayNextMonth: null
    property real endOffset: firstWeekDayNextMonth ? (height - firstWeekDayNextMonth.y) : 0

    delegate: Item {
        id: dayRoot
        implicitWidth: Math.max(dateDelegateMetrics.width, dateDelegateMetrics.height) * 2
        implicitHeight: width
        onYChanged: {
            if (monthGrid.month < model.month || (monthGrid.month === Calendar.December && model.month === Calendar.January)) {
                if (model.day === 1) {
                    monthGrid.firstNextMonth = this
                }
                if (index % 7 && model.day >= 1 && model.day <= 7) {
                    monthGrid.firstWeekDayNextMonth = this
                }
            }
        }
        TextMetrics {
            id: dateDelegateMetrics
            text: "00"
            font.pixelSize: 12
        }
        Column {
            id: dateColumn
            opacity: monthGrid.month === model.month ? 1.0 : 0.0
            anchors.centerIn: parent
            Text {
                color: (calendarComponent.month - 1) === model.month ? mainPalette.text : mainPalette.borderColor
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: 12
                font.bold: model.today
                text: model.day
            }
            Text {
                visible: model.day === 1
                color: "red"
                text: monthGrid.title.slice(0, 3).toUpperCase()
                font.pixelSize: 12
            }
        }
        Rectangle {
            z: -1
            anchors.centerIn: dateColumn
            height: parent.height + 3
            width: height
            color: Qt.rgba(50/255.0, 215/255.0, 75/255.0, 1.0)
            opacity: calendarComponent.day === model.day && (calendarComponent.month - 1) === model.month ? 1.0 :
                                                                                   dayMouse.containsMouse ? 0.5 : 0.0
            Behavior on opacity {
                NumberAnimation {
                    duration: 120
                }
            }

            radius: 4
            MouseArea {
                id: dayMouse
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    console.log("Day " + model.day + " clicked")
                    var date = String(model.year)+"."+String(model.month+1).padStart(2, '0')+"."+String(model.day).padStart(2, '0')
                    console.warn((new Date(date).getTime() / 1000).toFixed(0))
                    var timestamp = (new Date(date).getTime() / 1000).toFixed(0)
                    toggl.setTimeEntryDate(modelData.GUID, timestamp)
                }
            }
        }
    }
}
