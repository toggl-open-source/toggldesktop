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

    @IBOutlet weak var backgroundBox: NSBox?
    @IBOutlet weak var foregroundBox: NSBox!
    
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

    private func handleMouseEntered(_ event: NSEvent) {
        guard isResizable else { return }

        // Skip exit if the user is resizing
        if isUserResizing && userAction != .none {
            return
        }

        delegate?.timelineCellMouseDidEntered(self)
    }

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

    private func handleMouseExit(_ event: NSEvent) {

        // Skip exit if the user is resizing
        if isUserResizing && userAction != .none {
            return
        }

        mousePosition = .none
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
        let localPosition = convertToLocalPoint(for: event)

        // Click action
        if userAction == .none && view.bounds.contains(localPosition) {
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

    private func convertToLocalPoint(for event: NSEvent) -> CGPoint {
        // Convert mouse location to local
        let position = event.locationInWindow
        let localPosition = foregroundBox.convert(position, from: nil)
        return localPosition
    }
}
