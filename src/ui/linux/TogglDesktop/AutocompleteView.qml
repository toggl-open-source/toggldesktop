import QtQuick 2.12
import QtQml.Models 2.12

Rectangle {
    id: root
    color: palette.base
    height: 500

    property string filter: ""

    ListView {
        anchors.fill: parent
        clip: true
        model: toggl.minitimerAutocomplete
        delegate: Loader {
            // TODO use the enum instead of magic values
            sourceComponent: autocompleteData.Type === 13 ? workspaceDelegate :
                             autocompleteData.Type === 12 ? clientDelegate :
                             autocompleteData.Type === 11 ? headerDelegate :
                                                            regularDelegate
            property var autocompleteData: modelData

            visible: autocompleteData.Description.includes(root.filter)
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

        Text {
            height: 24
            x: 9
            textFormat: Text.RichText
            property string timeEntry: autocompleteData.Description
            property string project: autocompleteData.ProjectLabel.length > 0 ?
                                         "<font color=" + autocompleteData.ProjectColor + "> • " + autocompleteData.ProjectLabel + "</font>" :
                                         ""
            property string task: autocompleteData.TaskLabel.length > 0 ? " " + autocompleteData.TaskLabel : ""
            property string client: autocompleteData.ClientLabel.length > 0 ? " " + autocompleteData.ClientLabel : ""
            text: timeEntry + project + task + client
        }
    }
}
