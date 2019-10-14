import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Layouts 1.12

import QtQuick.Controls 2.12

Window {
    id: window
    visible: true
    minimumWidth: 360
    width: 360
    minimumHeight: 640
    height: 640

    property bool loggingIn: !loginSwitch.checked
    property bool signingUp: !loggingIn

    Rectangle {
        anchors.fill: parent
        color: "#202020"
    }

    TextMetrics {
        id: termsAndConditionsMetrics
        text: "Agree to conditions and terms TBD"
    }

    Column {
        y: parent.height / 5
        anchors.horizontalCenter: parent.horizontalCenter
        width: termsAndConditionsMetrics.width + 30
        spacing: 18
        add: Transition {
            NumberAnimation {
                properties: "opacity"
                from: 0.0
                to: 1.0
                duration: 120
            }
            NumberAnimation {
                properties: "x,y"
                duration: 0
            }
        }
        move: Transition {
            NumberAnimation {
                properties: "y"
                duration: 120
            }
        }

        Image {
            anchors.horizontalCenter: parent.horizontalCenter
            source: "qrc:/images/logo.png"
        }

        Item { height: 3; width: 1 }

        RowLayout {
            width: parent.width
            Text {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                text: "Login"
                color: loggingIn ? "white" : "#9a9a9a"
            }
            Switch {
                id: loginSwitch
                onCheckedChanged: username.forceActiveFocus()
            }
            Text {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                text: "Sign up"
                color: signingUp ? "white;" : "#9a9a9a"
            }
        }

        RowLayout {
            width: parent.width
            visible: signingUp
            Button {
                id: signupWithEmail
                Layout.fillWidth: true
                checkable: true
                autoExclusive: true
                checked: true
                text: "With email"
            }
            Button {
                id: signupWithGoogle
                Layout.fillWidth: true
                checkable: true
                autoExclusive: true
                text: "With Google"
            }
        }

        TextField {
            width: parent.width
            id: username
            focus: true
            anchors.horizontalCenter: parent.horizontalCenter
            placeholderText: "Username"
            visible: loggingIn || !signupWithGoogle.checked
        }

        TextField {
            width: parent.width
            id: password
            anchors.horizontalCenter: parent.horizontalCenter
            placeholderText: "Password"
            echoMode: TextField.Password
            visible: loggingIn || !signupWithGoogle.checked
        }

        ComboBox {
            id: country
            width: parent.width
            anchors.horizontalCenter: parent.horizontalCenter
            visible: signingUp
            model: [ "United States", "Estonia", "Czechia" ]
            currentIndex: -1
            displayText: currentIndex < 0 ? "Please select your country" : currentText
        }

        Row {
            id: termsAndConditionsRow
            width: parent.width
            visible: signingUp
            anchors.horizontalCenter: parent.horizontalCenter
            CheckBox {
                id: termsAndConditions
            }
            Text {
                id: termsAndConditionsText
                anchors.verticalCenter: parent.verticalCenter
                text: "Agree to <a href=https://toggl.com>conditions and terms TBD</a>"
                color: "#d2d2d2"
            }
        }

        Button {
            width: parent.width
            anchors.horizontalCenter: parent.horizontalCenter
            text: signingUp ? "Sign up" : "Log in"
            enabled: signingUp ? (!signupWithEmail.checked || (username.text.length > 0 && password.text.length > 0)) &&
                                  country.currentIndex > -1 && termsAndConditions.checked
                               : username.text.length > 0 && password.text.length > 0
            onClicked: loggingIn ? toggl.login(username.text, password.text) : toggl.signup(username.text, password.text, country.currentIndex)
        }
    }

    Rectangle {
        id: errorOverlay
        width: parent.width
        color: "red"
        height: 48
        y: -height
        Behavior on y { NumberAnimation { duration: 120 } }
        Connections {
            target: toggl
            onDisplayError: {
                errorOverlay.y = 0
                errorText.text = errmsg
            }
        }
        RowLayout {
            anchors.fill: parent
            anchors.margins: 6
            Text {
                id: errorText
                Layout.fillHeight: true
                Layout.fillWidth: true
                verticalAlignment: Text.AlignVCenter
            }
            Button {
                text: "x"
                onClicked: errorOverlay.y = -errorOverlay.height
                implicitWidth: implicitHeight
            }
        }
    }
}
