//
//  ResizablePopover.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 6/25/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

class ResizablePopover: NSPopover {

    let BOTTOM_HIT = CGFloat(4)
    let CORNER_HIT = CGFloat(10)

    private enum Region {
        case None
        case Left
        case LeftBottom
        case Right
        case RightBottom
    }

    // MARK: Variables

    private var min: NSSize
    private var max: NSSize
    private var region: Region = .None
    private var down: NSPoint?
    private var size: NSSize?
    private var trackLeft: NSView.TrackingRectTag?
    private var trackRight: NSView.TrackingRectTag?
    private var trackLeftBottom: NSView.TrackingRectTag?
    private var trackRightBottom: NSView.TrackingRectTag?
    private var trackBottom: NSView.TrackingRectTag?
    private var sizeChanged: ((_ size: NSSize) -> Void)? = nil

    private let cursorLeftBottom = NSCursor.resizeLeftRight
    private let cursorRightBottom = NSCursor.resizeUpDown

    override var contentViewController: NSViewController? {
        didSet {
            prepareTracker()
        }
    }
    // min: the mimimum size the popover is allowed to be
    // max: the maximum size the popover is allowed to be
    // heightFromBottom: Set this to limit the draggable handle region
    //                   to a height starting from the bottom of the popover
    //                   If 0, then it uses the entire height of popover
    public init(min: NSSize, max: NSSize) {
        self.min = min
        self.max = max

        // If not defined, use the screen height
        if let screen = NSScreen.main,
            self.max.height == 0 {
            self.max.height = CGFloat(screen.frame.height)
        }
        super.init()
    }

    required public init?(coder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }

    deinit {
        clearTrackers()
    }

    // Call this to get notified anytime the popover is resized
    func resized(_ sizeChanged: @escaping (_ size: NSSize) -> Void) {
        self.sizeChanged = sizeChanged
    }

    private func prepareTracker() {
        guard let controller = contentViewController else { return }

        // Set default content size
        contentSize = NSSize(width: controller.view.bounds.width, height: controller.view.bounds.height)

        // Setup the tracker
        setTrackers()
    }

    override public func mouseEntered(with event: NSEvent) {
        if region == .None {
            if event.trackingNumber == trackLeft {
                region = .Left
            } else if event.trackingNumber == trackRight {
                region = .Right
            } else if event.trackingNumber == trackLeftBottom {
                region = .LeftBottom
            } else if event.trackingNumber == trackRightBottom {
                region = .RightBottom
            } else {
                region = .None
            }

            setCursor()
        }
    }

    override public func mouseExited(with event: NSEvent) {
        if down == nil {
            region = .None
            setCursor()
        }
    }

    override public func mouseDown(with event: NSEvent) {
        self.size = contentSize
        self.down = NSEvent.mouseLocation
    }

    override public func mouseDragged(with event: NSEvent) {
        if region == .None {
            return
        }

        guard let size = size else { return }
        guard let down = down else { return }

        let location = NSEvent.mouseLocation

        var movedX = (location.x - down.x) * 2
        let movedY = location.y - down.y
        //        print("MOVE x: \(movedX), y: \(movedY)")

        if region == .Left || region == .LeftBottom {
            movedX = -movedX
        }

        var newWidth = size.width + movedX
        if newWidth < min.width {
            newWidth = min.width
        } else if newWidth > max.width {
            newWidth = max.width
        }

        var newHeight = size.height - movedY
        if newHeight < min.height {
            newHeight = min.height
        } else if newHeight > max.height {
            newHeight = max.height
        }

        switch region {
        case .Left: fallthrough
        case .Right:
            contentSize = NSSize(width: newWidth, height: contentSize.height)
        case .LeftBottom: fallthrough
        case .RightBottom:
            contentSize = NSSize(width: newWidth, height: newHeight)
        default:
            break
        }

        setCursor()
    }

    override public func mouseUp(with event: NSEvent) {
        if region != .None {
            region = .None
            setCursor()
            setTrackers()
            down = nil

            if let onChanged = sizeChanged {
                onChanged(NSSize(width: contentSize.width, height: contentSize.height))
            }
        }
    }

    private func setCursor() {
        switch region {
        case .Left:
            NSCursor.resizeLeft.set()
        case .Right:
            NSCursor.resizeRight.set()
        case .LeftBottom:
            cursorLeftBottom.set()
        case .RightBottom:
            cursorRightBottom.set()
        default:
            NSCursor.arrow.set()
        }
    }

    private func setTrackers() {
        clearTrackers()

        if let view = contentViewController?.view {
            var bounds = NSRect(x: 0, y: CORNER_HIT, width: SIDES_HIT, height: CORNER_HIT)
            trackLeft = view.addTrackingRect(bounds, owner: self, userData: nil, assumeInside: false)

            bounds = NSRect(x: contentSize.width - SIDES_HIT, y: CORNER_HIT, width: SIDES_HIT, height: CORNER_HIT)
            trackRight = view.addTrackingRect(bounds, owner: self, userData: nil, assumeInside: false)

            bounds = NSRect(x: 0, y: 0, width: CORNER_HIT, height: CORNER_HIT)
            trackLeftBottom = view.addTrackingRect(bounds, owner: self, userData: nil, assumeInside: false)

            bounds = NSRect(x: contentSize.width - CORNER_HIT, y: 0, width: CORNER_HIT, height: CORNER_HIT)
            trackRightBottom = view.addTrackingRect(bounds, owner: self, userData: nil, assumeInside: false)

            bounds = NSRect(x: CORNER_HIT, y: 0, width: contentSize.width - CORNER_HIT * 2, height: BOTTOM_HIT)
            trackBottom = view.addTrackingRect(bounds, owner: self, userData: nil, assumeInside: false)
        }
    }

    private func clearTrackers() {
        if let view = contentViewController?.view, let left = trackLeft, let right = trackRight, let leftBottom = trackLeftBottom, let rightBottom = trackRightBottom, let bottom = trackBottom {
            view.removeTrackingRect(left)
            view.removeTrackingRect(right)
            view.removeTrackingRect(rightBottom)
            view.removeTrackingRect(leftBottom)
            view.removeTrackingRect(bottom)
        }
    }

    private static func getCursor(_ name: String) -> NSCursor {
        return NSCursor.openHand
        let path = Bundle(for: self).bundlePath

        let image = NSImage(byReferencingFile: path + "/Resources/resources/\(name)_cursor.pdf")
        let info = NSDictionary(contentsOfFile: path + "/Resources/resources/\(name)_info.plist")
        let cursor = NSCursor(image: image!, hotSpot: NSPoint(x: (info!.value(forKey: "hotx")! as AnyObject).doubleValue!, y: (info!.value(forKey: "hoty")! as AnyObject).doubleValue!))

        return cursor
    }
}
