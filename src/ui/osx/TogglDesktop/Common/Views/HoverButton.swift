//
//  HoverButton.swift
//  TogglDesktop
//
//  Created by Andrew Nester on 25.11.2020.
//  Copyright © 2020 Toggl. All rights reserved.
//

import Foundation

class HoverButton: KeyButton {

    var hoverImage: NSImage?
    private var originalImage: NSImage?

    override var state: NSControl.StateValue {
        didSet {
            if state == .off {
                originalImage = nil
            }
        }
    }

    override func awakeFromNib() {
        super.awakeFromNib()

        let trackingArea = NSTrackingArea(rect: NSRect.zero,
                                          options: [
                                            .activeInActiveApp,
                                            .mouseEnteredAndExited,
                                            .assumeInside,
                                            .inVisibleRect
                                          ],
                                          owner: self)
        addTrackingArea(trackingArea)

        setCanBecomeKeyView(true)
    }

    override func mouseEntered(with event: NSEvent) {
        originalImage = image
        animator().image = hoverImage
    }

    override func mouseExited(with event: NSEvent) {
        if originalImage != nil {
            animator().image = originalImage
        }
    }

    override func mouseDown(with event: NSEvent) {
        state = .on
        // not calling `super` so we can receive `mouseUp` event
    }

    override func mouseUp(with event: NSEvent) {
        super.mouseUp(with: event)
        guard isEnabled else { return }

        let mousePoint = convert(event.locationInWindow, from: nil)
        let isUpInside = bounds.contains(mousePoint)
        if isUpInside {
            sendAction(action, to: target)
        }

        state = .off
    }
}
