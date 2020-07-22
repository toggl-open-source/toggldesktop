//
//  NSView+Shadow+Border.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 3/4/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

extension NSView {

    @objc func applyShadow(color: NSColor = NSColor.black,
                     opacity: Float = 0.13,
                     radius: CGFloat = 6.0,
                     offset: CGSize = CGSize(width: 0, height: -2)) {
        wantsLayer = true
        layer?.masksToBounds = false
        shadow = NSShadow()
        layer?.shadowOpacity = opacity
        layer?.shadowColor = color.cgColor
        layer?.shadowOffset = offset
        layer?.shadowRadius = radius
    }

    func applyBorder(width: CGFloat = 1,
                     color: NSColor = NSColor(white: 0.0, alpha: 0.1),
                     cornerRadius: CGFloat = 8) {
        wantsLayer = true
        layer?.masksToBounds = false
        layer?.cornerRadius = cornerRadius
        layer?.borderWidth = width;
        layer?.borderColor = color.cgColor
    }
}
