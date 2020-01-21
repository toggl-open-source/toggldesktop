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
        static let SideHit: CGFloat = 10.0
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
    private var dragPoisition = MousePosition.none { didSet { print("dragPoisition = \(dragPoisition)") }}
    private var trackTop: NSView.TrackingRectTag?
    private var trackBottom: NSView.TrackingRectTag?
    private var trackMiddle: NSView.TrackingRectTag?

    // MARK: Public

    override func viewDidLoad() {
        super.viewDidLoad()
    }

    override func mouseEntered(with event: NSEvent) {
        mouseDelegate?.timelineCellMouseDidEntered(self)

        // Determine which drag position is
        switch event.trackingNumber {
        case trackTop:
            if isResizable {
                dragPoisition = .top
            }
        case trackBottom:
            if isResizable {
                dragPoisition = .bottom
            }
        case trackMiddle:
            if isClickable {
                dragPoisition = .middle
            }
        default:
            dragPoisition = .none
        }

        // Set cursor
        updateCursor()
    }

    override func mouseExited(with event: NSEvent) {
        super.mouseExited(with: event)
        mouseDelegate?.timelineCellMouseDidExited(self)

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

    func initAllTracking() {
        clearResizeTrackers()

        initHoverTrackers()
        initResizeTrackers()
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
        switch dragPoisition {
        case .top,
             .bottom:
            NSCursor.resizeUpDown.set()
        case .middle:
            NSCursor.pointingHand.set()
        case .none:
            NSCursor.arrow.set()
        }
    }
}
