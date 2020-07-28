//
//  NSImage+Tint.swift
//  TogglDesktop
//
//  Created by Andrew Nester on 28.07.2020.
//  Copyright © 2020 Toggl. All rights reserved.
//

import Foundation

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
