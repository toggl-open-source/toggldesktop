//
//  AutoCompleteRowView.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 4/3/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

final class AutoCompleteRowView: NSTableRowView {

    private let selectionColor = Color.autoCompleteCellHover.color

    override func drawSelection(in dirtyRect: NSRect) {
        let selectionRect = bounds.insetBy(dx: 5, dy: 2)
        selectionColor.setFill()
        let selectionPath = NSBezierPath(roundedRect: selectionRect, xRadius: 8, yRadius: 8)
        selectionPath.fill()
    }
}
