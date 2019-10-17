import QtQuick 2.12
import QtQuick.Layouts 1.12

Item {
    id: root
    height: 500

    property real maximumHeight: 500

    property var model: null
    property var currentItem: null

    property string filter: ""
    onFilterChanged: {
        toggl.minitimerAutocomplete.setFilter(filter)
        list.currentIndex = -1
    }
    onVisibleChanged: list.currentIndex = -1

    function upPressed() {
        console.log("==== START UP ====")
        console.log("Current index: " + list.currentIndex)
        console.log("There is " + list.count + " rows")
        console.log("Model reports " + model.count() + " rows")
        for (var i = list.currentIndex - 1; i > 0 ; --i) {
            if (model.get(i).Type < 10) {
                list.currentIndex = i
                list.positionViewAtIndex(i, ListView.Center)
                console.log("Current index: " + list.currentIndex)
                return
            }
        }
        console.log("Current index: " + list.currentIndex)
    }
    function downPressed() {
        console.log("==== START DOWN ====")
        console.log("Current index: " + list.currentIndex)
        console.log("There is " + list.count + " rows")
        console.log("Model reports " + model.count() + " rows")
        for (var i = list.currentIndex + 1; i < list.count; i++) {
            if (model.get(i).Type < 10) {
                list.currentIndex = i
                list.positionViewAtIndex(i, ListView.Center)
                console.log("Current index: " + list.currentIndex)
                return
            }
        }
        console.log("Current index: " + list.currentIndex)
    }

    Rectangle {
        anchors.fill: list
        anchors.margins: -1
        color: palette.dark
    }

    ListView {
        id: list
        width: parent.width
        height: root.maximumHeight
        clip: true
        model: root.model
        currentIndex: -1
        onCurrentIndexChanged: {
            console.log(currentIndex)
        }

        onCountChanged: {
            var base = list.visibleChildren[0]
            var listViewHeight = 0
            for (var i = 0; i < base.visibleChildren.length; i++)
                listViewHeight += base.visibleChildren[i].height
            list.height = Math.min(listViewHeight, root.maximumHeight)
        }
        highlightFollowsCurrentItem: true
        highlight: Rectangle {
            color: "red"
            width: root.width
            height: 24
        }
        delegate: Rectangle {
            width: root.width
            height: 24
            color: ListView.isCurrentItem ? "red" : palette.base
            property bool selectable: modelData.Type < 10
            Loader {
                // TODO use the enum instead of magic values
                sourceComponent: autocompleteData.Type === 13 ? workspaceDelegate :
                                 autocompleteData.Type === 12 ? clientDelegate :
                                 autocompleteData.Type === 11 ? headerDelegate :
                                                                regularDelegate
                property var autocompleteData: modelData
                property int index: index
            }
        }
    }

    Component {
        id: workspaceDelegate

        Text {
            height: 24
            width: root.width
            font.bold: true
            horizontalAlignment: Text.AlignHCenter
            color: "#9e9e9e"

            text: autocompleteData.Description
        }
    }

    Component {
        id: clientDelegate

        Text {
            height: 24
            x: 9
            color: "#9e9e9e"

            text: autocompleteData.Description
        }
    }

    Component {
        id: headerDelegate

        Text {
            height: 24
            text: autocompleteData.Description
            color: "#9e9e9e"
        }
    }

    Component {
        id: regularDelegate

        Item {
            height: 24
            width: root.width

            Text {
                anchors.fill: parent
                verticalAlignment: Text.AlignVCenter
                x: 9
                textFormat: Text.RichText
                property string timeEntry: autocompleteData.Description
                property string project: autocompleteData.ProjectLabel.length > 0 ?
                                             "<font color=" + autocompleteData.ProjectColor + "> • " + autocompleteData.ProjectLabel + "</font>" :
                                             ""
                property string task: autocompleteData.TaskLabel.length > 0 ? " " + autocompleteData.TaskLabel : ""
                property string client: autocompleteData.ClientLabel.length > 0 ? " " + autocompleteData.ClientLabel : ""
                text: (ListView.isCurrentItem ? "HOVNO" : "") + timeEntry + project + task + client
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
