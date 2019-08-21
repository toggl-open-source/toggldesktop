//
//  DotImageView.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 2/25/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

final class DotImageView: NSImageView {

    // MARK: Variables
    
    private let icon = NSImage(named: NSImage.Name("time-entry-dot"))!

    // MARK: Public

    @objc func fill(with tintColor: NSColor) {
        let visibleColor = tintColor.visibleColor()
        let iconWithColor = icon.image(withTintColor: visibleColor)
        image = iconWithColor
    }
}

extension NSImage {

    func image(withTintColor tintColor: NSColor) -> NSImage {
        guard isTemplate else { return self }
        guard let copiedImage = self.copy() as? NSImage else { return self }
        copiedImage.lockFocus()
        tintColor.set()
        let imageBounds = NSRect(x: 0, y: 0, width: copiedImage.size.width, height: copiedImage.size.height)
        imageBounds.fill(using: .sourceAtop)
        copiedImage.unlockFocus()
        copiedImage.isTemplate = false
        return copiedImage
    }
}
