//
//  HoverImageView.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 1/3/20.
//  Copyright © 2020 Alari. All rights reserved.
//

import Cocoa

final class HoverImageView: NSImageView {

    // MARK: Variables


    // MARK: View

    override func awakeFromNib() {
        super.awakeFromNib()
        initTrackingArea()
    }

    // MARK: Public

    override func mouseEntered(with event: NSEvent) {
        image = NSImage(named: "timeline-activity-recorder-info-icon-hover")
    }

    override func mouseExited(with event: NSEvent) {
        image = NSImage(named: "timeline-activity-recorder-info-icon")
    }
}

// MARK: Private

extension HoverImageView {

    private func initTrackingArea() {
        if trackingAreas.isEmpty {
            let tracking = NSTrackingArea(rect: bounds, options: [.mouseEnteredAndExited, .activeInActiveApp, .inVisibleRect], owner: self, userInfo: nil)
            addTrackingArea(tracking)
            updateTrackingAreas()
        }
    }
}
