//
//  TimelineTimeEntryCell.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 6/21/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

protocol TimelineTimeEntryCellDelegate: class {

    func timeEntryCellMouseDidEntered(_ sender: TimelineTimeEntryCell)
    func timeEntryCellMouseDidExited(_ sender: TimelineTimeEntryCell)
}

final class TimelineTimeEntryCell: NSCollectionViewItem {

    // MARK: OUTLET

    @IBOutlet weak var backgroundView: NSBox!
    @IBOutlet weak var topOffset: NSLayoutConstraint!
    @IBOutlet weak var bottomOffset: NSLayoutConstraint!

    // MARK: Variables

    weak var delegate: TimelineTimeEntryCellDelegate?
    private(set) var timeEntry: TimelineTimeEntry!
    private var trackingArea: NSTrackingArea?
    private lazy var timeEntryMenu = TimelineTimeEntryMenu()

    // MARK: View

    override func viewDidLoad() {
        super.viewDidLoad()

        initTrackingArea()
    }

    override func prepareForReuse() {
        super.prepareForReuse()

        if let trackingArea = trackingArea {
            backgroundView.removeTrackingArea(trackingArea)
            self.trackingArea = nil
        }
        initTrackingArea()
    }

    // MARK: Public

    func config(for timeEntry: TimelineTimeEntry, at zoomLevel: TimelineDatasource.ZoomLevel) {
        self.timeEntry = timeEntry
        backgroundView.fillColor = timeEntry.color

        if timeEntry.isSmall {
            backgroundView.cornerRadius = 1.0
            topOffset.constant = 0
            bottomOffset.constant = 0
        } else {
            // Adjust the top and bottom
            // So those timeentry will not collide each other
            var gap = zoomLevel.minimumGap
            if (gap * 2.0) >= view.frame.height {
                gap = 1.0
            }
            topOffset.constant = gap
            bottomOffset.constant = gap

            // If the size is too smal
            // It's better to reduce the corner radius
            if view.frame.height <= 20.0 {
                backgroundView.cornerRadius = 6
            } else {
                backgroundView.cornerRadius = 10.0
            }
        }
    }

    override func mouseEntered(with event: NSEvent) {
        super.mouseEntered(with: event)
        delegate?.timeEntryCellMouseDidEntered(self)
    }

    override func mouseExited(with event: NSEvent) {
        super.mouseExited(with: event)
        delegate?.timeEntryCellMouseDidExited(self)
    }

    override func rightMouseDown(with event: NSEvent) {
        super.rightMouseDown(with: event)

        if let timeEntry = timeEntry, timeEntry.isOverlap {
            NSMenu.popUpContextMenu(timeEntryMenu, with: event, for: self.view)
        }
    }
}

extension TimelineTimeEntryCell {

    fileprivate func initTrackingArea() {
        let tracking = NSTrackingArea(rect: view.bounds, options: [.mouseEnteredAndExited, .activeInActiveApp, .inVisibleRect], owner: self, userInfo: nil)
        self.trackingArea = tracking
        backgroundView.addTrackingArea(tracking)
        backgroundView.updateTrackingAreas()
    }
}
