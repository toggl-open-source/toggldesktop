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

    @IBInspectable var bgColor: NSColor?
    @IBInspectable var titleColor: NSColor?
    @IBInspectable var cornerRadius: CGFloat = 0 {
        didSet {
            wantsLayer = true
            layer?.cornerRadius = cornerRadius
        }
    }

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
            let attributes: [NSAttributedString.Key: Any] = [
                NSAttributedString.Key.foregroundColor: titleColor,
                NSAttributedString.Key.font: font,
                NSAttributedString.Key.paragraphStyle: style]
            let attributedTitle = NSAttributedString(string: title, attributes: attributes)
            self.attributedTitle = attributedTitle
        }
    }
}
