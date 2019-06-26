//
//  NoVibrantPopoverView.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 4/19/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

class NoVibrantPopoverView: ResizablePopover {

    override var appearance: NSAppearance? {
        get {
            if let appearance = NSApplication.shared.windows.first?.effectiveAppearance {
                if appearance.name.rawValue.lowercased().contains("dark") {
                    if #available(OSX 10.14, *) {
                        return NSAppearance(named: .darkAqua)
                    } else {
                        return NSAppearance(named: .aqua)
                    }
                }
            }
            return NSAppearance(named: .aqua)
        }
        set {}
    }

    @objc func present(from rect: NSRect, of view: NSView, preferredEdge: NSRectEdge = .maxX) {
        show(relativeTo: rect, of: view, preferredEdge: preferredEdge)
    }

    @objc func close(focusTimer: Bool) {

        // Close and notify delegate if need
        performClose(self)
    }
}
