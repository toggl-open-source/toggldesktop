//
//  FlatButton.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 1/28/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

@IBDesignable
class FlatButton: NSButton {

    // MARK: IBInspectable
    @IBInspectable var bgColor: NSColor?
    @IBInspectable var titleColor: NSColor?
    @IBInspectable var selectedTextColor: NSColor?
    @IBInspectable var cornerRadius: CGFloat = 0 {
        didSet {
            wantsLayer = true
            layer?.cornerRadius = cornerRadius
        }
    }
    @IBInspectable var borderColor: NSColor? {
        didSet {
            wantsLayer = true
            layer?.borderColor = borderColor?.cgColor
        }
    }
    @IBInspectable var borderWidth: CGFloat = 0 {
        didSet {
            wantsLayer = true
            layer?.borderWidth = borderWidth
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

    override func layout() {
        super.layout()

        layer?.borderColor = borderColor?.cgColor
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

    override func drawFocusRingMask() {
        let rect = self.bounds
        rect.fill()
    }

    private func drawTextColor() {
        if let titleColor = titleColor, let font = font {
            let style = NSMutableParagraphStyle()
            style.alignment = .center
            let selectedColor = self.selectedTextColor ?? titleColor
            let drawTextColor = isSelected ? selectedColor : titleColor
            let attributes: [NSAttributedString.Key: Any] = [
                NSAttributedString.Key.foregroundColor: drawTextColor,
                NSAttributedString.Key.font: font,
                NSAttributedString.Key.paragraphStyle: style]
            let attributedTitle = NSAttributedString(string: title, attributes: attributes)
            self.attributedTitle = attributedTitle
        }
    }
}
