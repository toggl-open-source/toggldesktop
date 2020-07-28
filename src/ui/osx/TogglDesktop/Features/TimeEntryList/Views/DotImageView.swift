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
        let visibleColor = tintColor.getAdaptiveColorForShape()
        let iconWithColor = icon.image(withTintColor: visibleColor)
        image = iconWithColor
    }
}
