//
//  HoverTableCellView.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 4/5/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

class HoverTableCellView: NSTableCellView {

    private var trackingArea: NSTrackingArea?

    // MARK: OUTLET

    @IBOutlet weak var contentContainerView: NSBox!

    // MARK: Public

    override func awakeFromNib() {
        super.awakeFromNib()
        contentContainerView.alphaValue = 0
        enableTracking()
    }

    override func prepareForReuse() {
        super.prepareForReuse()
        NSCursor.arrow.set()
        contentContainerView.alphaValue = 0.0
        enableTracking()
    }

    override func mouseExited(with event: NSEvent) {
        super.mouseExited(with: event)
        NSCursor.arrow.set()
        contentContainerView.animator().alphaValue = 0.0
    }

    override func mouseEntered(with event: NSEvent) {
        super.mouseEntered(with: event)
        NSCursor.pointingHand.set()
        contentContainerView.animator().alphaValue = 1.0
    }

    fileprivate func enableTracking() {
        if let trackingArea = trackingArea {
            removeTrackingArea(trackingArea)
        }
        let tracking = NSTrackingArea(rect: bounds,
                                      options: [.activeInActiveApp, .inVisibleRect, .mouseEnteredAndExited],
                                      owner: self,
                                      userInfo: nil)
        addTrackingArea(tracking)
        trackingArea = tracking
    }
}
