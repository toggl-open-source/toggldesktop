//
//  FlatButton.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 1/28/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

@IBDesignable
final class FlatButton: NSButton {

    // MARK: IBInspectable

    @IBInspectable var bgColor: NSColor?
<<<<<<< HEAD:src/ui/osx/TogglDesktop/test2/FlatButton.swift
    @IBInspectable var textColor: NSColor?
=======
    @IBInspectable var titleColor: NSColor?
>>>>>>> feature/timeline-ui:src/ui/osx/TogglDesktop/Swift/Views/FlatButton.swift
    @IBInspectable var selectedTextColor: NSColor?
    @IBInspectable var cornerRadius: CGFloat = 0 {
        didSet {
            wantsLayer = true
            layer?.cornerRadius = cornerRadius
        }
    }

    // MARK: Variables

    var isSelected = false {
        didSet {
            drawTextColor()
        }
    }

    // MARK: View

    override func awakeFromNib() {
        super.awakeFromNib()

        drawTextColor()
    }

    override func draw(_ dirtyRect: NSRect) {
        if let bgColor = bgColor {
            bgColor.setFill()
            __NSRectFill(dirtyRect)
        }

        drawTextColor()

        super.draw(dirtyRect)
    }

    private func drawTextColor() {
        if let titleColor = titleColor, let font = font {
            let style = NSMutableParagraphStyle()
            style.alignment = .center
<<<<<<< HEAD:src/ui/osx/TogglDesktop/test2/FlatButton.swift
            let selectedColor = self.selectedTextColor ?? textColor
            let drawTextColor = isSelected ? selectedColor : textColor
=======
            let selectedColor = self.selectedTextColor ?? titleColor
            let drawTextColor = isSelected ? selectedColor : titleColor
>>>>>>> feature/timeline-ui:src/ui/osx/TogglDesktop/Swift/Views/FlatButton.swift
            let attributes: [NSAttributedString.Key: Any] = [
                NSAttributedString.Key.foregroundColor: drawTextColor,
                NSAttributedString.Key.font: font,
                NSAttributedString.Key.paragraphStyle: style]
            let attributedTitle = NSAttributedString(string: title, attributes: attributes)
            self.attributedTitle = attributedTitle
        }
    }
}
