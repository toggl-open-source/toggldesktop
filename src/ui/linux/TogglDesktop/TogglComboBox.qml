import QtQuick 2.12
import QtQuick.Controls 2.12

ComboBox {
    id: control
    background: TogglButtonBackground {
        control: control
    }
    indicator: Canvas {
        id: canvas
        x: control.width - width - control.rightPadding
        y: control.topPadding + (control.availableHeight - height) / 2
        width: 10
        height: 5
        contextType: "2d"

        Connections {
            target: control
            onPressedChanged: canvas.requestPaint()
        }

        onPaint: {
            context.reset();
            context.moveTo(0, 0);
            context.lineTo(width, 0);
            context.lineTo(width / 2, height);
            context.closePath();
            context.fillStyle = mainPalette.buttonText;
            context.fill();
        }
    }
    contentItem: Text {
        leftPadding: 0
        rightPadding: control.indicator.width + control.spacing

        text: control.displayText
        font: control.font
        color: control.currentIndex < 0 ? disabledPalette.buttonText : mainPalette.buttonText
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
}
