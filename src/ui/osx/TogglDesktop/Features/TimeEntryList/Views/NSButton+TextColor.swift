//
//  NSButton+TextColor.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 2/25/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

extension NSButton {

    @objc func setTextColor(_ color: NSColor) {
        var attributes: [NSAttributedString.Key: Any] = [:]

        // Font
        let font = self.font ?? NSFont.systemFont(ofSize: 13.0)
        attributes[.font] = font

        // Color
        attributes[.foregroundColor] = color

        // Paragraph
        let paragraph = NSMutableParagraphStyle()
        paragraph.alignment = alignment
        attributes[.paragraphStyle] = paragraph

        attributedTitle = NSAttributedString(string: self.title, attributes: attributes)
    }
}
