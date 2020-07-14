//
//  TimelineBackgroundCell.swift
//  TogglTrack
//
//  Created by Nghia Tran on 12/13/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

/// CollectionViewItem to draw a background with different colors
/// It works with CollectionViewFlowLayout
final class TimelineBackgroundCell: NSCollectionViewItem {

    override func loadView() {
        self.view = TimelineBackgroundView()
    }
}

/// Responsible for drawing the background of the Timeline View
/// From the design, each hour will have different colors
final class TimelineBackgroundView: NSView {

    // It's crucial to pass the mouse action to the above view
    // If not, the Timeline Entry couldn't receive the MouseDown for example
    override func hitTest(_ point: NSPoint) -> NSView? {
        return nil
    }

    private var highlightColor: NSColor {
        if #available(OSX 10.13, *) {
            return NSColor(named: NSColor.Name("timeline-background-highlight-color"))!
        } else {
            return ConvertHexColor.hexCode(toNSColor: "#f7f7f7")
        }
    }

    override func draw(_ dirtyRect: NSRect) {
        super.draw(dirtyRect)
        highlightColor.setFill()
        dirtyRect.fill()
    }
}
