//
//  PopoverRootView.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 4/18/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

@objc final class PopoverRootView: NSView {

    override func viewDidMoveToWindow() {
        if let view = window?.contentView?.superview {
            let backgroundView = PopoverBackgroundView(frame: NSRect.zero)
            view.addSubview(backgroundView, positioned: NSWindow.OrderingMode.below, relativeTo: view)
            backgroundView.edgesToSuperView()
        }

        super.viewDidMoveToWindow()
    }
}

final class PopoverBackgroundView: NSView {

    var backgroundColor: NSColor {
        if #available(OSX 10.13, *) {
            return NSColor(named: NSColor.Name("white-background-color"))!
        } else {
            return NSColor.white
        }
    }

    override func draw(_ dirtyRect: NSRect) {
        self.backgroundColor.set()
        dirtyRect.fill()
    }
}
