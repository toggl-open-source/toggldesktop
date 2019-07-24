//
//  ResizablePopover.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 6/25/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

class ResizablePopover: NSPopover {

    private struct Constants {
        static let CornerHit: CGFloat = 10.0
        static let SideHit: CGFloat = 4.0
    }

    private enum Region {
        case None
        case Left
        case Right
    }

    // MARK: Variables

    private var isResizable: Bool
    private var min: NSSize
    private var max: NSSize
    private var size: NSSize?
    private var region: Region = .None
    private var down: NSPoint?
    private var bottomHeight = CGFloat(20)
    private var trackLeft: NSView.TrackingRectTag?
    private var trackRight: NSView.TrackingRectTag?
    override var contentViewController: NSViewController? {
        didSet {
            prepareTracker()
        }
    }

    // MARK: Init

    override init() {
        self.isResizable = false
        self.min = CGSize.zero
        self.max = CGSize.zero
        super.init()
    }

    init(min: CGSize, max: CGSize) {
        self.isResizable = true
        self.min = min
        self.max = max

        // If not defined, use the screen height
        if let screen = NSScreen.main {
            if max.height == 0 {
                self.max.height = CGFloat(screen.frame.height)
            }
            if max.width == 0 {
                self.max.width = CGFloat(screen.frame.width)
            }
        }
        super.init()
    }

    required public init?(coder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }

    deinit {
        clearTrackers()
    }

    // MARK: Override

    override public func mouseEntered(with event: NSEvent) {
        guard isResizable else {
            super.mouseEntered(with: event)
            return
        }

        if region == .None {
            if event.trackingNumber == trackLeft {
                region = .Left
            } else if event.trackingNumber == trackRight {
                region = .Right
            } else {
                region = .None
            }

            setCursor()
        }
    }

    override public func mouseExited(with event: NSEvent) {
        guard isResizable else {
            super.mouseExited(with: event)
            return
        }

        if down == nil {
            region = .None
            setCursor()
        }
    }

    override public func mouseDown(with event: NSEvent) {
        guard isResizable else {
            super.mouseDown(with: event)
            return
        }

        self.size = contentSize
        self.down = NSEvent.mouseLocation
    }

    override public func mouseDragged(with event: NSEvent) {
        guard isResizable else {
            super.mouseDragged(with: event)
            return
        }

        if region == .None {
            return
        }

        guard let size = size else { return }
        guard let down = down else { return }

        let location = NSEvent.mouseLocation
        var movedX = location.x - down.x
        if region == .Left {
            movedX = -movedX
        }

        var newWidth = size.width + movedX
        if newWidth < min.width {
            newWidth = min.width
        } else if newWidth > max.width {
            newWidth = max.width
        }

        switch region {
        case .Left,
             .Right:
            contentSize = NSSize(width: newWidth, height: contentSize.height)
        default:
            break
        }

        setCursor()
    }

    override public func mouseUp(with event: NSEvent) {
        guard isResizable else {
            super.mouseUp(with: event)
            return
        }

        if region != .None {
            region = .None
            setCursor()
            setTrackers()
            down = nil
            popoverDidResize()
        }
    }

    func popoverDidResize() {
        // for overriden
    }

    // MARK: Private

    private func prepareTracker() {
        guard let controller = contentViewController else { return }

        // Set default content size
        contentSize = NSSize(width: controller.view.bounds.width, height: controller.view.bounds.height)
        bottomHeight = contentSize.height

        // Setup the tracker
        setTrackers()
    }

    private func setCursor() {
        switch region {
        case .Left,
             .Right:
            NSCursor.resizeLeftRight.set()
        default:
            NSCursor.arrow.set()
        }
    }

    private func setTrackers() {
        guard isResizable else { return }
        clearTrackers()

        if let view = contentViewController?.view {
            var bounds = NSRect(x: 0, y: Constants.CornerHit, width: Constants.SideHit, height: bottomHeight - Constants.CornerHit)
            trackLeft = view.addTrackingRect(bounds, owner: self, userData: nil, assumeInside: false)

            bounds = NSRect(x: contentSize.width - Constants.SideHit, y: Constants.CornerHit, width: Constants.SideHit, height: bottomHeight - Constants.CornerHit)
            trackRight = view.addTrackingRect(bounds, owner: self, userData: nil, assumeInside: false)
        }
    }

    private func clearTrackers() {
        if let view = contentViewController?.view, let left = trackLeft, let right = trackRight {
            view.removeTrackingRect(left)
            view.removeTrackingRect(right)
        }
    }
}
