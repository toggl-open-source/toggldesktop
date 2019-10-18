import QtQuick 2.12
import QtQuick.Layouts 1.12

import QtQuick.Controls 2.12

Item {
    anchors.fill: parent

    property bool loggingIn: !loginSwitch.checked
    property bool signingUp: !loggingIn

    property bool everythingFilled: signingUp ? (!signupWithEmail.checked || (username.text.length > 0 && password.text.length > 0)) &&
                                                 country.currentIndex > -1 && termsAndConditions.checked
                                              : username.text.length > 0 && password.text.length > 0

    function act() {
        if (!everythingFilled)
            return
        if (loggingIn)
            toggl.login(username.text, password.text)
        else if (signingUp) {
            if (signupWithEmail.checked)
                toggl.signup(username.text, password.text, country.selectedID)
            else if (signupWithGoogle.checked) {
                // don't do anything in this case
            }
        }
    }

    property real shadowWidth: 9

    Rectangle {
        anchors.fill: loginColumn
        anchors.margins: -24


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
                GradientStop { position: 0.5; color: "light gray" }
                GradientStop { position: 1.0; color: palette.alternateBase }
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
                GradientStop { position: 0.5; color: "light gray" }
                GradientStop { position: 0.0; color: palette.alternateBase }
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
                GradientStop { position: 0.5; color: "light gray" }
                GradientStop { position: 1.0; color: palette.alternateBase }
            }
        }

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
                GradientStop { position: 0.5; color: "light gray" }
                GradientStop { position: 0.0; color: palette.alternateBase }
            }
        }

        Rectangle {
            anchors.right: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom

            width: shadowWidth
            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop { position: 0.0; color: palette.alternateBase }
                GradientStop { position: 1.0; color: "light gray" }
            }
        }

        Rectangle {
            anchors.left: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom

            width: shadowWidth
            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop { position: 0.0; color: "light gray" }
                GradientStop { position: 1.0; color: palette.alternateBase }
            }
        }
        Rectangle {
            anchors.top: parent.bottom
            anchors.right: parent.right
            anchors.left: parent.left

            height: shadowWidth
            gradient: Gradient {
                GradientStop { position: 0.0; color: "light gray" }
                GradientStop { position: 1.0; color: palette.alternateBase }
            }
        }
        Rectangle {
            anchors.bottom: parent.top
            anchors.right: parent.right
            anchors.left: parent.left

            height: shadowWidth
            gradient: Gradient {
                GradientStop { position: 0.0; color: palette.alternateBase }
                GradientStop { position: 1.0; color: "light gray" }
            }
        }
    }

    Column {
        id: loginColumn
        y: parent.height / 6.7
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
            width: parent.width * 0.67
            fillMode: Image.PreserveAspectFit
            antialiasing: true
            mipmap: true
        }

        Item {
            width: parent.width
            height: loginSwitch.height

            Text {
                anchors.right: loginSwitch.left
                anchors.rightMargin: 9
                anchors.verticalCenter: parent.verticalCenter
                text: "Login"
                color: loggingIn ? palette.text : disabledPalette.text
            }
            TogglSwitch {
                id: loginSwitch
                anchors.horizontalCenter: parent.horizontalCenter
                Layout.alignment: Qt.AlignCenter
                onCheckedChanged: username.forceActiveFocus()
            }
            Text {
                anchors.left: loginSwitch.right
                anchors.leftMargin: 9
                anchors.verticalCenter: parent.verticalCenter
                text: "Sign up"
                color: signingUp ? palette.text : disabledPalette.text
            }
        }

        RowLayout {
            width: parent.width
            visible: signingUp
            TogglButton {
                id: signupWithEmail
                Layout.fillWidth: true
                checkable: true
                autoExclusive: true
                checked: true
                text: "With email"
            }
            TogglButton {
                id: signupWithGoogle
                Layout.fillWidth: true
                checkable: true
                autoExclusive: true
                text: "With Google"
            }
        }

        TogglTextField {
            width: parent.width
            id: username
            focus: true
            anchors.horizontalCenter: parent.horizontalCenter
            placeholderText: "Username"
            visible: loggingIn || !signupWithGoogle.checked
            onAccepted: act()
        }

        TogglTextField {
            width: parent.width
            id: password
            anchors.horizontalCenter: parent.horizontalCenter
            placeholderText: "Password"
            echoMode: TextField.Password
            visible: loggingIn || !signupWithGoogle.checked
            onAccepted: act()
        }

        TogglComboBox {
            id: country
            width: parent.width
            anchors.horizontalCenter: parent.horizontalCenter
            visible: signingUp
            model: toggl.countries
            textRole: "Text"
            currentIndex: -1
            property int selectedID: toggl.countries && toggl.countries[currentIndex] ? toggl.countries[currentIndex].ID : -1
            displayText: currentIndex < 0 ? "Please select your country" : currentText
        }

        RowLayout {
            id: termsAndConditionsRow
            width: parent.width
            visible: signingUp
            anchors.horizontalCenter: parent.horizontalCenter
            TogglCheckBox {
                id: termsAndConditions
            }
            Text {
                id: termsAndConditionsText
                anchors.verticalCenter: parent.verticalCenter
                Layout.fillWidth: true
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                font.pointSize: 9
                text: "I agree to <a href=\"https://toggl.com/legal/terms/\">terms of service</a> and <a href=\"https://toggl.com/legal/privacy/\">privacy policy</a>"
                color: palette.text
            }
        }

        TogglButton {
            width: parent.width
            anchors.horizontalCenter: parent.horizontalCenter
            text: signingUp ? "Sign up" : "Log in"
            enabled: everythingFilled
            onClicked: act()
        }

        TogglButton {
            width: parent.width
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Log in with Google"
            visible: loggingIn
            enabled: false
        }
    }
}
