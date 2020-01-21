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
}

class TimelineBaseCell: NSCollectionViewItem {

    private struct Constants {
        static let SideHit: CGFloat = 5.0
    }

    private enum DragPosition {
        case top
        case bottom
        case none
    }

    // MARK: OUTLET

    @IBOutlet weak var backgroundBox: NSBox?
    @IBOutlet weak var foregroundBox: NSBox!
    
    // MARK: Variables

    weak var mouseDelegate: TimelineBaseCellDelegate?
    private(set) var backgroundColor: NSColor?
    var isResizable: Bool { return false }

    // Resizable tracker
    private var dragPoisition = DragPosition.none
    private var trackTop: NSView.TrackingRectTag?
    private var trackBottom: NSView.TrackingRectTag?

    // MARK: Public

    override func viewDidLoad() {
        super.viewDidLoad()
        initTrackingArea()
    }

    override func mouseEntered(with event: NSEvent) {
        mouseDelegate?.timelineCellMouseDidEntered(self)

        // Skip if it's not resizable
        guard isResizable else {
            super.mouseEntered(with: event)
            return
        }

        // Determine which drag position is
        guard dragPoisition == .none else { return }
        switch event.trackingNumber {
        case trackTop:
            dragPoisition = .top
        case trackBottom:
            dragPoisition = .bottom
        default:
            dragPoisition = .none
        }

        // Set cursor
        updateCursor()
    }

    override func mouseExited(with event: NSEvent) {
        mouseDelegate?.timelineCellMouseDidExited(self)

        // Skip if it's not resizable
        guard isResizable else {
            super.mouseEntered(with: event)
            return
        }

        // Reset
        dragPoisition = .none
        updateCursor()
    }

    func renderColor(with foregroundColor: NSColor, isSmallEntry: Bool) {
        backgroundColor = foregroundColor.lighten(by: 0.1)

        foregroundBox.fillColor = foregroundColor
        backgroundBox?.fillColor = backgroundColor ?? foregroundColor
        backgroundBox?.borderColor = backgroundColor ?? foregroundColor

        let cornerRadius = suitableCornerRadius(isSmallEntry)
        foregroundBox.cornerRadius = cornerRadius
        backgroundBox?.cornerRadius = cornerRadius
    }

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

    func initTrackingArea() {
        if foregroundBox.trackingAreas.isEmpty {
            let tracking = NSTrackingArea(rect: view.bounds, options: [.mouseEnteredAndExited, .activeInActiveApp, .inVisibleRect], owner: self, userInfo: nil)
            foregroundBox.addTrackingArea(tracking)
            foregroundBox.updateTrackingAreas()
        }
    }
}

// MARK: Resizable

extension TimelineBaseCell {

    private func initResizeTrackers() {
        guard let view = foregroundBox, isResizable else { return }
        clearResizeTrackers()

        var bounds = NSRect(x: 0, y: Constants.SideHit, width: view.bounds.width, height: Constants.SideHit)
        trackTop = view.addTrackingRect(bounds, owner: self, userData: nil, assumeInside: false)

        bounds = NSRect(x: 0, y: view.bounds.width - Constants.SideHit, width: view.bounds.width, height: Constants.SideHit)
        trackBottom = view.addTrackingRect(bounds, owner: self, userData: nil, assumeInside: false)
    }

    private func clearResizeTrackers() {
        guard let view = foregroundBox,
            let trackTop = trackTop,
            let trackBottom = trackBottom else {
            return
        }
        view.removeTrackingRect(trackTop)
        view.removeTrackingRect(trackBottom)
    }

    private func updateCursor() {
        switch dragPoisition {
        case .top:
            NSCursor.resizeUp.set()
        case .bottom:
            NSCursor.resizeDown.set()
        default:
            NSCursor.arrow.set()
        }
    }
}
