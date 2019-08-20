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

    // MARK: OUTLET

    @IBOutlet weak var backgroundView: NSBox!

    // MARK: Variables

    weak var mouseDelegate: TimelineBaseCellDelegate?
    private var trackingArea: NSTrackingArea?

    // MARK: Public

    override func prepareForReuse() {
        super.prepareForReuse()

        if let trackingArea = trackingArea {
            backgroundView.removeTrackingArea(trackingArea)
            self.trackingArea = nil
        }
        initTrackingArea()
    }

    override func mouseEntered(with event: NSEvent) {
        super.mouseEntered(with: event)
        mouseDelegate?.timelineCellMouseDidEntered(self)
    }

    override func mouseExited(with event: NSEvent) {
        super.mouseExited(with: event)
        mouseDelegate?.timelineCellMouseDidExited(self)
    }

    func renderBackground(with color: NSColor, isSmallEntry: Bool) {
        backgroundView.fillColor = color
        if isSmallEntry {
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

    func initTrackingArea() {
        let tracking = NSTrackingArea(rect: view.bounds, options: [.mouseEnteredAndExited, .activeInActiveApp, .inVisibleRect], owner: self, userInfo: nil)
        self.trackingArea = tracking
        backgroundView.addTrackingArea(tracking)
        backgroundView.updateTrackingAreas()
    }
}
