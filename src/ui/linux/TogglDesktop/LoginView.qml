import QtQuick 2.12
import QtQuick.Layouts 1.12

import QtQuick.Controls 2.12

Item {
    anchors.fill: parent

    property bool loggingIn: true
    property bool signingUp: !loggingIn

    property bool everythingFilled: {
        if (loggingIn) {
            return username.filled && password.filled
        }
        else {
            return username.filled && password.filled && country.filled && termsAndConditions.filled
        }
    }

    function act() {
        if (!everythingFilled)
            return
        if (loggingIn)
            toggl.login(username.text, password.text)
        else if (signingUp) {
            toggl.signup(username.text, password.text, country.selectedID)
        }
    }

    property real shadowWidth: 9

    Rectangle {
        anchors.fill: parent
        color: "white" // TODO
    }

    Rectangle {
        anchors.fill: loginColumn
        anchors.margins: -18
        color: mainPalette.base
        radius: 8
    }

    Column {
        id: loginColumn
        y: parent.height / 6.7
        anchors.horizontalCenter: parent.horizontalCenter
        width: 196
        spacing: 12
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
            width: parent.width * 0.33
            fillMode: Image.PreserveAspectFit
            antialiasing: true
            mipmap: true
        }

        /*
        Item {
            width: parent.width
            height: loginSwitch.height

            Text {
                anchors.right: loginSwitch.left
                anchors.rightMargin: 9
                anchors.verticalCenter: parent.verticalCenter
                text: "Login"
                color: loggingIn ? mainPalette.text : disabledPalette.text
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
                color: signingUp ? mainPalette.text : disabledPalette.text
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
        */

        TogglTextField {
            width: parent.width
            id: username
            focus: true
            anchors.horizontalCenter: parent.horizontalCenter
            placeholderText: qsTr("Email address")
            property bool filled: text.length > 0 // change this to a regexp eventually
            onAccepted: act()
        }

        Column {
            width: parent.width
            spacing: 6
            TogglTextField {
                width: parent.width
                id: password
                anchors.horizontalCenter: parent.horizontalCenter
                placeholderText: "Password"
                echoMode: TextField.Password
                property bool filled: text.length > 0
                onAccepted: act()
            }

            Text {
                visible: loggingIn
                width: parent.width
                text: qsTr("Forgot password?")
                font.underline: true
                color: mainPalette.borderColor
                MouseArea {
                    anchors.fill: parent
                    anchors.margins: -2
                    cursorShape: Qt.PointingHandCursor
                    hoverEnabled: true
                }
            }
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
            property bool filled: selectedID >= 0
            displayText: currentIndex < 0 ? "Please select your country" : currentText
        }

        RowLayout {
            id: termsAndConditionsRow
            width: parent.width
            visible: signingUp
            anchors.horizontalCenter: parent.horizontalCenter
            TogglCheckBox {
                id: termsAndConditions
                property bool acceptable: checked
            }
            Text {
                id: termsAndConditionsText
                Layout.alignment: Qt.AlignVCenter
                Layout.fillWidth: true
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                font.pixelSize: 9
                text: "I agree to <a href=\"https://toggl.com/legal/terms/\">terms of service</a> and <a href=\"https://toggl.com/legal/privacy/\">privacy policy</a>"
                color: mainPalette.text
            }
        }

        TogglButton {
            width: parent.width
            anchors.horizontalCenter: parent.horizontalCenter
            text: signingUp ? "Sign up" : "Log in"
            color: Qt.rgba(229/255.0, 85/255.0, 98/255.0, 1.0)
            textColor: "white"
            enabled: everythingFilled
            onClicked: act()
        }

        TogglButton {
            width: parent.width
            anchors.horizontalCenter: parent.horizontalCenter
            text: signingUp ? "Sign up with Google" : "Log in with Google"
            visible: loggingIn
            enabled: false
        }

        Column {
            spacing: 0
            width: parent.width
            Text {
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                visible: loggingIn
                text: "Don't have an account?"
                color: mainPalette.borderColor
            }

            Text {
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                font.underline: true
                text: loggingIn ? "Sign up for free" : "Back to Log in"
                color: mainPalette.borderColor
                MouseArea {
                    anchors.fill: parent
                    anchors.margins: -2
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: loggingIn = !loggingIn
                }
            }
        }
    }
}
