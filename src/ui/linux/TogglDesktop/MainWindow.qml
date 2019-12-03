import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Layouts 1.12

import QtQuick.Controls 1.4

Rectangle {
    id: mainWindow
    visible: true
    width: 400
    height: 640

    function mixColors(a, b, ratio) {
        return Qt.rgba(
            ratio * a.r + (1.0 - ratio) * b.r,
            ratio * a.g + (1.0 - ratio) * b.g,
            ratio * a.b + (1.0 - ratio) * b.b,
            ratio * a.a + (1.0 - ratio) * b.a,
        )
    }

    function setAlpha(color, alpha) {
        return Qt.rgba(color.r, color.g, color.b, alpha)
    }

    SystemPalette {
        id: palette
        property bool isDark: (itemShadow.r + itemShadow.g + itemShadow.b) < 1

        property int itemShadowSize: palette.isDark ? 1 : 9
        property color itemShadow: mixColors(palette.shadow, palette.listBackground, 0.2)
        property color listBackground: mixColors(palette.base, palette.alternateBase, 0.8)
    }
    SystemPalette {
        id: disabledPalette
        colorGroup: SystemPalette.Disabled
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
        anchors.fill: parent
    }

    TimeEntryEditView {
        id: timeEntryEdit
        visible: false
        anchors.fill: parent
    }
}
