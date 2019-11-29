import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Layouts 1.12

import QtQuick.Controls 1.4

ApplicationWindow {
    id: window
    visible: true
    minimumWidth: 400
    width: 400
    minimumHeight: 640
    height: 640
    title: "Toggl"

    function mixColors(a, b, ratio) {
        return Qt.rgba(
            ratio * a.r + (1.0 - ratio) * b.r,
            ratio * a.g + (1.0 - ratio) * b.g,
            ratio * a.b + (1.0 - ratio) * b.b,
            ratio * a.a + (1.0 - ratio) * b.a,
        )
    }

    SystemPalette {
        id: palette
        property bool isDark: (shadowColor.r + shadowColor.g + shadowColor.b) < 300
    }
    SystemPalette {
        id: disabledPalette
        colorGroup: SystemPalette.Disabled
    }

    menuBar: MenuBar {
        Menu {
            title: "Toggl Desktop"
            MenuItem {
                text: "Log out"
                onTriggered: toggl.logout()
            }
            MenuItem {
                text: "Exit"
                onTriggered: Qt.quit()
            }
        }
    }

    Rectangle {
        anchors.fill: parent
        color: palette.base
        clip: true

        ErrorOverlay {
            width: parent.width
            height: 48
        }
    }

    TextMetrics {
        id: termsAndConditionsMetrics
        font.pointSize: 9
        text: "I agree to terms of service and privacy policy"
    }

    Connections {
        target: toggl
        onDisplayLogin: {
            if (open) {
                mainView.source = "LoginView.qml"
                timeEntryEdit.visible = false
            }
        }
        onDisplayTimeEntryList: {
            if (open) {
                mainView.source = "TimeEntryListView.qml"
                timeEntryEdit.visible = false
            }
        }
        onDisplayTimeEntryEditor: {
            if (open) {
                timeEntryEdit.timeEntry = view
                timeEntryEdit.visible = true
            }
        }
    }

    Loader {
        id: mainView
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: timeEntryEdit.visible ? timeEntryEdit.left : parent.right
    }

    TimeEntryEditView {
        id: timeEntryEdit
        visible: false
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        width: 360
        onVisibleChanged: {
            if (visible)
                window.width += width
            else
                window.width -= width
        }
    }

}
