//
//  PanelSwitcherButton.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 9/4/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

final class PanelSwitcherButton: FlatButton {

    // MARK: Variables

    private var trackingArea: NSTrackingArea?
    private lazy var backgroundColor: NSColor = {
        if #available(OSX 10.13, *) {
            return NSColor(named: NSColor.Name("white-background-hover-color"))!
        } else {
            return ConvertHexColor.hexCode(toNSColor: "#F8F8F8")
        }
    }()

    // MARK: View Cycle

    override func awakeFromNib() {
        super.awakeFromNib()
        initTrackingArea()
    }

    // MARK: Overriden

    override func mouseEntered(with event: NSEvent) {
        super.mouseEntered(with: event)
        bgColor = backgroundColor
        setNeedsDisplay()
    }

    override func mouseExited(with event: NSEvent) {
        super.mouseExited(with: event)
        bgColor = nil
        setNeedsDisplay()
    }

    private func initTrackingArea() {
        let tracking = NSTrackingArea(rect: bounds, options: [.mouseEnteredAndExited, .activeInActiveApp, .inVisibleRect], owner: self, userInfo: nil)
        self.trackingArea = tracking
        addTrackingArea(tracking)
        updateTrackingAreas()
    }
}
