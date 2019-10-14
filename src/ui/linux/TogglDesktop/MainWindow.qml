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
    }

    TextMetrics {
        id: termsAndConditionsMetrics
        text: "Agree to conditions and terms TBD"
    }

    Connections {
        target: toggl
        onDisplayLogin: {
            if (open)
                mainView.source = "LoginView.qml"
        }
        onDisplayTimeEntryList: {
            if (open)
                mainView.source = "TimeEntryListView.qml"
        }
    }

    Loader {
        id: mainView
        anchors.fill: parent
    }

    ErrorOverlay {
        width: parent.width
        height: 48
    }
}
