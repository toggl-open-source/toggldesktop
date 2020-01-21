//
//  TimelineBaseCell.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 8/20/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

protocol TimelineBaseCellDelegate: class {

    func timelineCellMouseDidEntered(_ sender: TimelineBaseCell)
    func timelineCellMouseDidExited(_ sender: TimelineBaseCell)
    func timelineCellRedrawEndTime(with event: NSEvent, sender: TimelineBaseCell)
    func timelineCellUpdateEndTime(with event: NSEvent, sender: TimelineBaseCell)
}

class TimelineBaseCell: NSCollectionViewItem {

    private struct Constants {
        static let SideHit: CGFloat = 20.0
    }

    private enum MousePosition {
        case top
        case bottom
        case middle
        case none
    }

    // MARK: OUTLET

    @IBOutlet weak var backgroundBox: NSBox?
    @IBOutlet weak var foregroundBox: NSBox!
    
    // MARK: Variables

    weak var mouseDelegate: TimelineBaseCellDelegate?
    private(set) var backgroundColor: NSColor?
    var isResizable: Bool { return false }
    var isClickable: Bool { return false }

    // Resizable tracker
    private var mousePosition = MousePosition.none { didSet { print("dragPoisition = \(mousePosition)") }}
    private var trackTop: NSView.TrackingRectTag?
    private var trackBottom: NSView.TrackingRectTag?
    private var trackMiddle: NSView.TrackingRectTag?
    private var mouseDownPoint: CGPoint?
    private var isResizing: Bool { return mousePosition == .top || mousePosition == .bottom }

    // MARK: View cycle

    override func viewDidLoad() {
        super.viewDidLoad()
    }

    // MARK: Mouse activity

    override func mouseEntered(with event: NSEvent) {
        print("mouseEntered")
        handleMouseEntered(event)
    }

    override func mouseExited(with event: NSEvent) {
        print("mouseExited")
        handleMouseExit(event)

    }

    override func mouseDown(with event: NSEvent) {
        print("mouseDown")
        handleMouseDownForResize(event)
    }

    override func mouseDragged(with event: NSEvent) {
        print("mouseDragged")
        handleMouseDraggedForResize(event)
    }

    override func mouseUp(with event: NSEvent) {
        print("mouseUp")
        handleMouseUpForResize(event)
    }

    // MARK: Public

    func renderColor(with foregroundColor: NSColor, isSmallEntry: Bool) {
        backgroundColor = foregroundColor.lighten(by: 0.1)

        foregroundBox.fillColor = foregroundColor
        backgroundBox?.fillColor = backgroundColor ?? foregroundColor
        backgroundBox?.borderColor = backgroundColor ?? foregroundColor

        let cornerRadius = suitableCornerRadius(isSmallEntry)
        foregroundBox.cornerRadius = cornerRadius
        backgroundBox?.cornerRadius = cornerRadius
    }

    func initAllTracking() {
        // Clear and init
        clearResizeTrackers()
        initHoverTrackers()
        initResizeTrackers()
    }
}

// MARK: Private

extension TimelineBaseCell {

    private func suitableCornerRadius(_ isSmallEntry: Bool) -> CGFloat {
        if isSmallEntry {
            return 1
        }

        // If the size is too smal
        // It's better to reduce the corner radius
        let height = view.frame.height
        switch height {
        case 0...2: return 1
        case 2...5: return 2
        case 5...20: return 5
        default:
            return 10
        }
    }
}

// MARK: Resizable

extension TimelineBaseCell {

    private func initHoverTrackers() {
        guard let view = foregroundBox, isClickable else { return }
        let bounds = NSRect(x: 0, y: Constants.SideHit, width: view.frame.width, height: view.frame.height - Constants.SideHit * 2)
        trackMiddle = view.addTrackingRect(bounds, owner: self, userData: nil, assumeInside: false)
    }

    private func initResizeTrackers() {
        guard let view = foregroundBox, isResizable else { return }

        var bounds = NSRect(x: 0, y: view.frame.height - Constants.SideHit, width: view.frame.width, height: Constants.SideHit)
        trackTop = view.addTrackingRect(bounds, owner: self, userData: nil, assumeInside: false)

        bounds = NSRect(x: 0, y: 0, width: view.frame.width, height: Constants.SideHit)
        trackBottom = view.addTrackingRect(bounds, owner: self, userData: nil, assumeInside: false)
    }

    private func clearResizeTrackers() {
        if let trackTop = trackTop {
            view.removeTrackingRect(trackTop)
        }
        if let trackBottom = trackBottom {
            view.removeTrackingRect(trackBottom)
        }
        if let trackMiddle = trackMiddle {
            view.removeTrackingRect(trackMiddle)
        }
    }

    private func updateCursor() {
        switch mousePosition {
        case .top,
             .bottom:
            NSCursor.resizeUpDown.set()
        case .middle:
            NSCursor.pointingHand.set()
        case .none:
            NSCursor.arrow.set()
        }
    }

    private func handleMouseEntered(_ event: NSEvent) {
        mouseDelegate?.timelineCellMouseDidEntered(self)

        // Determine which drag position is
        switch event.trackingNumber {
        case trackTop:
            if isResizable {
                mousePosition = .top
            }
        case trackBottom:
            if isResizable {
                mousePosition = .bottom
            }
        case trackMiddle:
            if isClickable {
                mousePosition = .middle
            }
        default:
            mousePosition = .none
        }

        // Set cursor
        updateCursor()
    }

    private func handleMouseExit(_ event: NSEvent) {
        mouseDelegate?.timelineCellMouseDidExited(self)

        // Skip exit if the user is resizing
        if isResizing && mouseDownPoint != nil {
            return
        }

        // Only Reset if the mouse is out of the foreground box
        let position = event.locationInWindow
        let localPosition = foregroundBox.convert(position, from: nil)
        if !foregroundBox.frame.contains(localPosition) {
            mousePosition = .none
            updateCursor()
        }
    }

    private func handleMouseDownForResize(_ event: NSEvent) {
        guard isResizable, isResizing else { return }
        mouseDownPoint = event.locationInWindow
    }

    private func handleMouseDraggedForResize(_ event: NSEvent) {
        guard isResizable, isResizing else { return }
        guard mouseDownPoint != nil else { return }
        mouseDelegate?.timelineCellRedrawEndTime(with: event, sender: self)
    }

    private func handleMouseUpForResize(_ event: NSEvent) {
        guard isResizable, isResizing else { return }
        if mouseDownPoint != nil {
            mouseDelegate?.timelineCellUpdateEndTime(with: event, sender: self)
        }

        mouseDownPoint = nil
        mousePosition = .none
        updateCursor()
    }
}
