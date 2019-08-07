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
            backgroundView.cornerRadius = 1
        } else {
            // If the size is too smal
            // It's better to reduce the corner radius
            let height = view.frame.height
            switch height {
            case 0...2: backgroundView.cornerRadius = 1
            case 2...5: backgroundView.cornerRadius = 2
            case 5...20: backgroundView.cornerRadius = 5
            default:
                backgroundView.cornerRadius = 10
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
