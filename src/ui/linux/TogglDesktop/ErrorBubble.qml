import QtQuick 2.12
import QtQuick.Layouts 1.12

import QtQuick.Controls 2.12


Column {
    id: messageLayout

    property real maximumWidth: Infinity
    width: maximumWidth

    function show(text) {
        errorComponent.createObject(messageLayout, { "text": text } )
    }

    move: Transition {
        NumberAnimation {
            properties: "y"
        }
    }
    add: Transition {
        NumberAnimation {
            properties: "x"
            from: messageLayout.width
            to: messageLayout.width - width
        }
    }

    Connections {
        target: toggl
        onDisplayError: {
            show(errmsg)
        }
    }

    Component {
        id: errorComponent
        Item {
            id: errorRoot
            Layout.preferredHeight: height
            Layout.preferredWidth: width
            Layout.alignment: Qt.AlignRight
            property alias text: errorText.text
            height: errorContainer.height + 18
            width: errorContainer.width + 18
            Rectangle {
                id: errorContainer
                color: mainPalette.base
                x: 9
                y: 9
                height: errorLayout.height + 12
                width: errorLayout.width + 12

                RowLayout {
                    id: errorLayout
                    x: 6
                    y: 6
                    Text {
                        id: errorText
                        verticalAlignment: Text.AlignVCenter
                        color: "red"
                    }

                    TogglButton {
                        id: closeButton
                        flat: true
                        implicitHeight: 15
                        implicitWidth: implicitHeight
                        Cross {
                            anchors.centerIn: parent
                        }
                        onClicked: errorRoot.destroy()
                    }
                }
                TogglShadowBox {
                    anchors.fill: parent
                    shadowWidth: mainPalette.itemShadowSize
                    shadowColor: mainPalette.itemShadow
                    backgroundColor: mainPalette.listBackground
                    sides: TogglShadowBox.Side.Left | TogglShadowBox.Side.Right | TogglShadowBox.Side.Bottom | TogglShadowBox.Side.Top
                }
            }
        }
    }
}
