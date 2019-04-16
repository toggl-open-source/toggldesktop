//
//  AutoCompleteRowView.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 4/3/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

final class AutoCompleteRowView: NSTableRowView {

    private lazy var selectionColor: NSColor = {
        if #available(OSX 10.13, *) {
            return NSColor(named: NSColor.Name("auto-complete-cell-hover"))!
        } else {
            return NSColor(white: 0.7, alpha: 0.2)
        }
    }()

    override func drawSelection(in dirtyRect: NSRect) {
        let selectionRect = NSInsetRect(self.bounds, 5, 2)
        selectionColor.setFill()
        let selectionPath = NSBezierPath(roundedRect: selectionRect, xRadius: 8, yRadius: 8)
        selectionPath.fill()
    }
}
