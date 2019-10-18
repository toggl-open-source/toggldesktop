import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Layouts 1.12

import QtQuick.Controls 1.4

ApplicationWindow {
    id: window
    visible: true
    minimumWidth: 360
    width: 360
    minimumHeight: 640
    height: 640

    SystemPalette {
        id: palette
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
        color: "#202020"
        clip: true

        ErrorOverlay {
            width: parent.width
            height: 48
        }
    }

    TextMetrics {
        id: termsAndConditionsMetrics
        text: "Agree to conditions and terms TBD"
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
