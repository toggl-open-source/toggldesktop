//
//  PanelSwitcherButton.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 9/4/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

/// Main Button [Time Entry | Timeline ]
/// Support Hover Effect
final class PanelSwitcherButton: FlatButton {

    // MARK: Variables

    private var trackingArea: NSTrackingArea?
    private let normalColor = Color.tabViewText.color
    private let selectedColor = NSColor.labelColor

    // MARK: View Cycle

    override func awakeFromNib() {
        super.awakeFromNib()
        initTrackingArea()
    }

    // MARK: Overriden

    override func mouseEntered(with event: NSEvent) {
        super.mouseEntered(with: event)
        titleColor = selectedColor
        setNeedsDisplay()
    }

    override func mouseExited(with event: NSEvent) {
        super.mouseExited(with: event)
        titleColor = normalColor
        setNeedsDisplay()
    }

    private func initTrackingArea() {
        let tracking = NSTrackingArea(rect: bounds, options: [.mouseEnteredAndExited, .activeInActiveApp, .inVisibleRect], owner: self, userInfo: nil)
        self.trackingArea = tracking
        addTrackingArea(tracking)
        updateTrackingAreas()
    }
}
