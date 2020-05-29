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

/// Parent cellof the Timeline Time Entry Cell
/// TODO: Remove this class, since there is only one subclass
class TimelineBaseCell: NSCollectionViewItem {

    private struct Constants {
        static let SideHit: CGFloat = 20.0
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

    @IBOutlet weak var backgroundBox: CornerBoxView?
    @IBOutlet weak var foregroundBox: CornerBoxView!
    
    // MARK: Variables

    weak var delegate: TimelineBaseCellDelegate?
    private(set) var backgroundColor: NSColor?
    var isResizable: Bool { return false }
    var isHoverable: Bool { return false }

    // Resizable tracker
    private var mousePosition = MousePosition.none { didSet { updateCursor() }}
    private var trackingArea: NSTrackingArea?
    private var userAction = UserAction.none
    private var isUserResizing: Bool { return mousePosition == .top || mousePosition == .bottom }

    // MARK: View cycle

    override func viewDidLoad() {
        super.viewDidLoad()
        initAllTracking()
    }

    override func prepareForReuse() {
        super.prepareForReuse()
    }

    // MARK: Mouse activity

    override func mouseEntered(with event: NSEvent) {
        handleMouseEntered(event)
    }

    override func mouseExited(with event: NSEvent) {
        handleMouseExit(event)
    }

    override func mouseDown(with event: NSEvent) {
        handleMouseDownForResize(event)
    }

    override func mouseDragged(with event: NSEvent) {
        handleMouseDraggedForResize(event)
    }

    override func mouseUp(with event: NSEvent) {
        handleMouseUpForResize(event)
    }

    // MARK: Public

    /// Render proper color for TimeEntry Cell
    /// - Parameters:
    ///   - foregroundColor: Foreground Color
    ///   - isSmallEntry: isSmallEntry
    func renderColor(with foregroundColor: NSColor, isSmallEntry: Bool) {
        // Get the lighter color like the design
        backgroundColor = foregroundColor.lighten(by: 0.2)

        // Set color and border
        foregroundBox.backgroundColor = foregroundColor
        backgroundBox?.backgroundColor = backgroundColor ?? foregroundColor
        backgroundBox?.borderColor = backgroundColor ?? foregroundColor

        // Find the suitable corner radius
        let cornerRadius = TimelineBaseCell.suitableCornerRadius(isSmallEntry, height: view.frame.height)
        foregroundBox.cornerRadius = cornerRadius
        backgroundBox?.cornerRadius = cornerRadius
    }

    func initAllTracking() {
        // Clear and init
        clearResizeTrackers()
        initHoverTrackers()
    }
}

// MARK: Private

extension TimelineBaseCell {

    /// Helper class to determine the best radius corner by the current height
    /// If the height is too small, but the radius corner is heigh, the result is bad
    /// - Parameters:
    ///   - isSmallEntry: Determine if it's small size
    ///   - height: Current height
    /// - Returns: Suitable corner radius
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
        guard isHoverable else { return }
        trackingArea = NSTrackingArea(rect: view.bounds, options: [.mouseEnteredAndExited, .activeAlways, .inVisibleRect, .mouseMoved], owner: self, userInfo: nil)
        view.addTrackingArea(trackingArea!)
    }

    private func clearResizeTrackers() {
        if let trackingArea = trackingArea {
            view.removeTrackingArea(trackingArea)
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

    /// Start dragging session
    /// - Parameter event: Mouse Event
    private func handleMouseEntered(_ event: NSEvent) {
        guard isResizable else { return }

        // Skip exit if the user is resizing
        if isUserResizing && userAction != .none {
            return
        }

        delegate?.timelineCellMouseDidEntered(self)
        super.mouseEntered(with: event)
    }

    /// Determine the mouse position depend on it position
    /// - Parameter event: Mouse Event
    override func mouseMoved(with event: NSEvent) {
        guard isResizable else { return }

        // Skip exit if the user is resizing
        if isUserResizing && userAction != .none {
            return
        }

        // Convert mouse location to local
        let localPosition = convertToLocalPoint(for: event)

        // Determine where the mouse is
        if suitableHoverRect().contains(localPosition) {
            mousePosition = .middle
        } else if suitableTopResizeRect().contains(localPosition) {
            mousePosition = .top
        } else if suitableBottomResizeRect().contains(localPosition) {
            mousePosition = .bottom
        }
    }

    /// Reset dragging session
    /// - Parameter event: Mouse Event
    private func handleMouseExit(_ event: NSEvent) {

        // Skip exit if the user is resizing
        if isUserResizing && userAction != .none {
            return
        }

        mousePosition = .none
        super.mouseExited(with: event)
    }

    /// Determine the user action when the mouse is down
    /// - Parameter event: Mouse Event
    private func handleMouseDownForResize(_ event: NSEvent) {
        guard isResizable, isUserResizing else {
            super.mouseDown(with: event)
            return
        }

        // Calculate the user action
        switch mousePosition {
        case .top:
            userAction = .resizeTop
        case .bottom:
            userAction = .resizeBottom
        default:
            userAction = .none
            super.mouseDown(with: event)
        }
    }

    /// Dragging session is starting
    /// Notify the UI to update the size
    /// - Parameter event: Mouse Event
    private func handleMouseDraggedForResize(_ event: NSEvent) {
        guard isResizable, isUserResizing, userAction != .none else {
            super.mouseDragged(with: event)
            return
        }

        // Update start / end depend on the user action
        switch userAction {
        case .resizeBottom:
            delegate?.timelineCellRedrawEndTime(with: event, sender: self)
        case .resizeTop:
            delegate?.timelineCellRedrawStartTime(with: event, sender: self)
        case .none:
            super.mouseDragged(with: event)
        }
    }

    /// Dragging session is end
    /// Time to notify the View to update in the library
    /// - Parameter event: Mouse Event
    private func handleMouseUpForResize(_ event: NSEvent) {
        let localPosition = convertToLocalPoint(for: event)

        // Click action
        if userAction == .none {
            if view.bounds.contains(localPosition) {
                delegate?.timelineCellOpenEditor(self)
            }
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
    }

    private var isSmallEntry: Bool {
        return (view.frame.height - Constants.SideHit * 3)  <= 0
    }

    /// Determine the middle area of the Time Entry to present the hover cursor
    /// - Returns: Frame of the middle area
    private func suitableHoverRect() -> CGRect {
        if isResizable {
            if isSmallEntry {
                return CGRect(x: 0, y: Constants.SideHideSmall, width: foregroundBox.frame.width, height: foregroundBox.frame.height - Constants.SideHideSmall * 2)
            }
            return NSRect(x: 0, y: Constants.SideHit, width: resizeView.frame.width, height: foregroundBox.frame.height - Constants.SideHit * 2)
        }
        return foregroundBox.bounds
    }

    /// Determine the top area of the Time Entry
    /// - Returns: Frame of the top area
    private func suitableTopResizeRect() -> CGRect {
        guard isResizable else { return .zero }
        if isSmallEntry {
            return NSRect(x: 0, y: foregroundBox.frame.height - Constants.SideHideSmall, width: foregroundBox.frame.width, height: Constants.SideHideSmall)
        }
        return NSRect(x: 0, y: foregroundBox.frame.height - Constants.SideHit, width: resizeView.frame.width, height: Constants.SideHit)
    }

    /// Determine the top area of the Time Entry
    /// - Returns: Frame of the top area
    private func suitableBottomResizeRect() -> CGRect {
        guard isResizable else { return .zero }
        if isSmallEntry {
            return NSRect(x: 0, y: 0, width: foregroundBox.frame.width, height: Constants.SideHideSmall)
        }
        return NSRect(x: 0, y: 0, width: resizeView.frame.width, height: Constants.SideHit)
    }

    /// Helper method to conver the mouse position to current cell
    /// - Parameter event: Mouse event
    /// - Returns: local position
    private func convertToLocalPoint(for event: NSEvent) -> CGPoint {
        // Convert mouse location to local
        let position = event.locationInWindow
        let localPosition = foregroundBox.convert(position, from: nil)
        return localPosition
    }

    private var resizeView: NSView {
        return backgroundBox ?? foregroundBox
    }
}
