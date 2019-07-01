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

    func config(for timeEntry: TimelineTimeEntry) {
        self.timeEntry = timeEntry
        backgroundView.fillColor = timeEntry.color
        backgroundView.cornerRadius = timeEntry.isSmall ? 1.0 : 10.0
    }

    @IBAction func timeEntryOnTap(_ sender: Any) {
    }

    override func mouseEntered(with event: NSEvent) {
        super.mouseEntered(with: event)
        delegate?.timeEntryCellMouseDidEntered(self)
    }

    override func mouseExited(with event: NSEvent) {
        super.mouseExited(with: event)
        delegate?.timeEntryCellMouseDidExited(self)
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
