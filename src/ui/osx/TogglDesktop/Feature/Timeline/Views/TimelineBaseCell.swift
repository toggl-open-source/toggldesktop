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
    func timelineCellRedrawEndTime(with event: NSEvent, sender: TimelineBaseCell)
    func timelineCellUpdateEndTime(with event: NSEvent, sender: TimelineBaseCell)
    func timelineCellRedrawStartTime(with event: NSEvent, sender: TimelineBaseCell)
    func timelineCellUpdateStartTime(with event: NSEvent, sender: TimelineBaseCell)
    func timelineCellOpenEditor(_ sender: TimelineBaseCell)
}

class TimelineBaseCell: NSCollectionViewItem {

    private struct Constants {
        static let SideHit: CGFloat = 10.0
        static let SideHideSmall: CGFloat = 4
    }

    private enum MousePosition {
        case top
        case bottom
        case middle
        case none
    }

    private enum UserAction {
        case resizeTop
        case resizeBottom
        case none
    }

    // MARK: OUTLET

    @IBOutlet weak var backgroundBox: NSBox?
    @IBOutlet weak var foregroundBox: NSBox!
    
    // MARK: Variables

    weak var delegate: TimelineBaseCellDelegate?
    private(set) var backgroundColor: NSColor?
    var isResizable: Bool { return false }
    var isHoverable: Bool { return false }

    // Resizable tracker
    private var mousePosition = MousePosition.none { didSet { print("dragPoisition = \(mousePosition)") }}
    private var trackTop: NSView.TrackingRectTag?
    private var trackBottom: NSView.TrackingRectTag?
    private var trackMiddle: NSView.TrackingRectTag?
    private var userAction = UserAction.none
    private var isUserResizing: Bool { return mousePosition == .top || mousePosition == .bottom }

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

        let cornerRadius = TimelineBaseCell.suitableCornerRadius(isSmallEntry, height: view.frame.height)
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

    class func suitableCornerRadius(_ isSmallEntry: Bool, height: CGFloat) -> CGFloat {
        if isSmallEntry {
            return 1
        }

        // If the size is too smal
        // It's better to reduce the corner radius
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
        guard let view = foregroundBox, isHoverable else { return }
        let bounds = suitableHoverRect()
        trackMiddle = view.addTrackingRect(bounds, owner: self, userData: nil, assumeInside: false)
    }

    private func initResizeTrackers() {
        guard let view = foregroundBox, isResizable else { return }

        var bounds = suitableTopResizeRect()
        trackTop = view.addTrackingRect(bounds, owner: self, userData: nil, assumeInside: false)

        bounds = suitableBottomResizeRect()
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
        delegate?.timelineCellMouseDidEntered(self)

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
            if isHoverable {
                mousePosition = .middle
            }
        default:
            mousePosition = .none
        }

        // Set cursor
        updateCursor()
    }

    private func handleMouseExit(_ event: NSEvent) {

        // Skip exit if the user is resizing
        if isUserResizing && userAction != .none {
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
        guard isResizable, isUserResizing else { return }

        // Calculate the user action
        switch mousePosition {
        case .top:
            userAction = .resizeTop
        case .bottom:
            userAction = .resizeBottom
        default:
            userAction = .none
        }
    }

    private func handleMouseDraggedForResize(_ event: NSEvent) {
        guard isResizable, isUserResizing else { return }
        guard userAction != .none else { return }

        // Update start / end depend on the user action
        switch userAction {
        case .resizeBottom:
            delegate?.timelineCellRedrawEndTime(with: event, sender: self)
        case .resizeTop:
            delegate?.timelineCellRedrawStartTime(with: event, sender: self)
        case .none:
            break
        }
    }

    private func handleMouseUpForResize(_ event: NSEvent) {

        // Click action
        if userAction == .none {
            print("Show Editor")
            delegate?.timelineCellOpenEditor(self)
        } else {
            // Dragging
            switch userAction {
            case .resizeBottom:
                delegate?.timelineCellUpdateEndTime(with: event, sender: self)
            case .resizeTop:
                delegate?.timelineCellUpdateStartTime(with: event, sender: self)
            case .none:
                break
            }
        }

        // Reset
        userAction = .none
        mousePosition = .none
        updateCursor()
    }

    private var isSmallEntry: Bool {
        return (view.frame.height - Constants.SideHit * 3)  <= 0
    }

    private func suitableHoverRect() -> CGRect {
        if isResizable {
            if isSmallEntry {
                return CGRect(x: 0, y: Constants.SideHideSmall, width: foregroundBox.frame.width, height: foregroundBox.frame.height - Constants.SideHideSmall * 2)
            }
            return NSRect(x: 0, y: Constants.SideHit, width: foregroundBox.frame.width, height: foregroundBox.frame.height - Constants.SideHit * 2)
        }
        return foregroundBox.bounds
    }

    private func suitableTopResizeRect() -> CGRect {
        guard isResizable else { return .zero }
        if isSmallEntry {
            return NSRect(x: 0, y: foregroundBox.frame.height - Constants.SideHideSmall, width: foregroundBox.frame.width, height: Constants.SideHideSmall)
        }
        return NSRect(x: 0, y: foregroundBox.frame.height - Constants.SideHit, width: foregroundBox.frame.width, height: Constants.SideHit)
    }

    private func suitableBottomResizeRect() -> CGRect {
        guard isResizable else { return .zero }
        if isSmallEntry {
            return NSRect(x: 0, y: 0, width: foregroundBox.frame.width, height: Constants.SideHideSmall)
        }
        return NSRect(x: 0, y: 0, width: foregroundBox.frame.width, height: Constants.SideHit)
    }
}
