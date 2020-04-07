import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 1.4

Item {
    id: root
    height: 500

    property real maximumHeight: 500

    property var model: null
    property QtObject currentItem: list.currentItem ? list.currentItem.autocompleteData : null

    property string filter: ""
    onFilterChanged: {
        toggl.minitimerAutocomplete.setFilter(filter)
        list.currentIndex = -1
    }
    onVisibleChanged: list.currentIndex = -1

    function upPressed() {
        //console.log("Current index: " + list.currentIndex)
        //console.log("There is " + list.count + " rows")
        //console.log("Model reports " + model.count() + " rows")
        for (var i = list.currentIndex - 1; i > 0 ; --i) {
            if (model.get(i).Type < 10) {
                list.currentIndex = i
                list.positionViewAtIndex(i, ListView.Center)
                //console.log("Current index: " + list.currentIndex)
                return
            }
        }
        //console.log("Current index: " + list.currentIndex)
    }
    function downPressed() {
        //console.log("Current index: " + list.currentIndex)
        //console.log("There is " + list.count + " rows")
        //console.log("Model reports " + model.count() + " rows")
        for (var i = list.currentIndex + 1; i < list.count; i++) {
            if (model.get(i).Type < 10) {
                list.currentIndex = i
                list.positionViewAtIndex(i, ListView.Center)
                //console.log("Current index: " + list.currentIndex)
                return
            }
        }
        //console.log("Current index: " + list.currentIndex)
    }

    Text {
        z: 20
        text: list.currentItem ? list.currentItem.autocompleteData.Description : ""
    }

    Rectangle {
        z: -1
        anchors.fill: parent
        color: setAlpha(mainPalette.shadow, 0.5)
    }

    ScrollView {
        id: scrollArea
        width: parent.width
        height: root.maximumHeight
        clip: true
        ListView {
            id: list
            model: root.model
            currentIndex: -1
            onCurrentIndexChanged: {
                console.log(currentIndex)
            }

            onCountChanged: {
                var base = list.visibleChildren[0]
                var listViewHeight = 0
                if (!base)
                    return
                for (var i = 0; i < base.visibleChildren.length; i++)
                    listViewHeight += base.visibleChildren[i].height
                list.height = Math.min(listViewHeight, root.maximumHeight)
            }
            highlightFollowsCurrentItem: true
            highlight: Rectangle {
                z: 30
                color: "light gray"
                opacity: 0.5
                width: root.width
                height: 24
            }
            delegate: Loader {
                // TODO use the enum instead of magic values
                sourceComponent: autocompleteData && autocompleteData.Type === 13 ? workspaceDelegate :
                                 autocompleteData && autocompleteData.Type === 12 ? clientDelegate :
                                 autocompleteData && autocompleteData.Type === 11 ? headerDelegate :
                                                                regularDelegate
                property var autocompleteData: modelData
                property int index: index
            }
            Rectangle {
                anchors {
                    left: parent.left
                    top: parent.top
                    right: parent.right
                }
                height: 2
                color: mainPalette.separator
            }
        }
    }

    Component {
        id: workspaceDelegate

        Rectangle {
            height: 40
            width: root.width
            color: mainPalette.base
            Text {
                anchors.centerIn: parent
                color: mainPalette.text

                text: autocompleteData && autocompleteData.Description
                font.pointSize: 12
            }
            Rectangle {
                anchors {
                    left: parent.left
                    bottom: parent.bottom
                    right: parent.right
                }
                height: 1
                color: mainPalette.separator
            }
        }
    }

    Component {
        id: clientDelegate

        Rectangle {
            height: 30
            width: root.width
            color: mainPalette.base
            Text {
                anchors.left: parent.left
                anchors.leftMargin: 24
                anchors.verticalCenter: parent.verticalCenter
                color: mainPalette.text

                text: autocompleteData && autocompleteData.Description
                font.pointSize: 10
            }
        }
    }

    Component {
        id: headerDelegate

        Rectangle {
            height: 30
            width: root.width
            color: mainPalette.base
            Text {
                anchors.left: parent.left
                anchors.leftMargin: 12
                anchors.verticalCenter: parent.verticalCenter
                text: autocompleteData && autocompleteData.Description
                color: mainPalette.text
                font.pointSize: 10
            }
        }
    }

    Component {
        id: regularDelegate

        Rectangle {
            height: 30
            width: root.width
            color: mainPalette.base

            Text {
                anchors.fill: parent
                anchors.left: parent.left
                anchors.leftMargin: 32
                verticalAlignment: Text.AlignVCenter
                textFormat: Text.RichText
                property string timeEntry: autocompleteData ? autocompleteData.Description : ""
                property string project: autocompleteData && autocompleteData.ProjectLabel.length > 0 ?
                                             "<font color=" + autocompleteData.ProjectColor + "> • " + autocompleteData.ProjectLabel + "</font>" :
                                             ""
                property string task: autocompleteData && autocompleteData.TaskLabel.length > 0 ? " " + autocompleteData.TaskLabel : ""
                text: (ListView.isCurrentItem ? "Iscurrent" : "") + timeEntry + project + task
                font.pointSize: 10
                font.weight: Font.Light
                color: mainPalette.text
            }
            MouseArea {
                id: delegateMouse
                anchors.fill: parent
                hoverEnabled: true
                onContainsMouseChanged: {
                }
            }
        }
    }
}
