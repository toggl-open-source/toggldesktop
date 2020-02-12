//
//  TimelineActivityCell.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 6/21/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

protocol TimelineActivityCellDelegate: class {

    func timelineActivityPresentPopover(_ sender: TimelineActivityCell)
}

final class TimelineActivityCell: NSCollectionViewItem {

    // MARK: OUTLET

    @IBOutlet weak var foregroundBox: NSBox!

    // MARK: Variables

    weak var delegate: TimelineActivityCellDelegate?
    private(set) var activity: TimelineActivity?

    // MARK: Public

    override func viewDidLoad() {
        super.viewDidLoad()
        initTrackingArea()
    }

    func config(for activity: TimelineActivity) {
        self.activity = activity
        let fgColor = activity.color
        renderColor(with: fgColor, isSmallEntry: activity.isSmall)
    }

    override func mouseEntered(with event: NSEvent) {
        super.mouseEntered(with: event)
        delegate?.timelineActivityPresentPopover(self)
    }
}

// MARK: Private

extension TimelineActivityCell {

    private func initTrackingArea() {
        if foregroundBox.trackingAreas.isEmpty {
            let tracking = NSTrackingArea(rect: view.bounds, options: [.mouseEnteredAndExited, .activeInActiveApp, .inVisibleRect], owner: self, userInfo: nil)
            foregroundBox.addTrackingArea(tracking)
            foregroundBox.updateTrackingAreas()
        }
    }

    private func renderColor(with foregroundColor: NSColor, isSmallEntry: Bool) {
        foregroundBox.fillColor = foregroundColor
        let cornerRadius = TimelineBaseCell.suitableCornerRadius(isSmallEntry, height: view.frame.height)
        foregroundBox.cornerRadius = cornerRadius
    }
}
