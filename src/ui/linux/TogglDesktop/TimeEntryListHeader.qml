import QtQuick 2.12

Item {
    Behavior on y {
        NumberAnimation {
            duration: 120
        }
    }

    Rectangle {
        y: sectionMargin
        x: 12
        width: timeEntryList.viewportWidth - 24
        height: headerHeight
        color: mainPalette.base

        TogglShadowBox {
            anchors.fill: parent
            shadowWidth: mainPalette.itemShadowSize
            shadowColor: mainPalette.itemShadow
            backgroundColor: mainPalette.listBackground
            sides: TogglShadowBox.Side.Left | TogglShadowBox.Side.Right | TogglShadowBox.Side.Top
        }

        Text {
            anchors.fill: parent
            anchors.leftMargin: 6
            anchors.rightMargin: 32
            textFormat: Text.RichText
            // this is actually rendering the width underneath so we force a repaint on each width change
            text: "<table width=100% cellpadding=6> " + section + "<tr><td colspan=2><font color=transparent>" + width + "</font></td></tr></table>"
            color: mainPalette.windowText
        }
    }
}
