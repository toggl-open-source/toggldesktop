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

    @IBOutlet weak var backgroundBox: NSBox?
    @IBOutlet weak var foregroundBox: NSBox!
    
    // MARK: Variables

    weak var mouseDelegate: TimelineBaseCellDelegate?

    // MARK: Public

    override func awakeFromNib() {
        super.awakeFromNib()
        initTrackingArea()
    }

    override func mouseEntered(with event: NSEvent) {
        mouseDelegate?.timelineCellMouseDidEntered(self)
    }

    override func mouseExited(with event: NSEvent) {
        mouseDelegate?.timelineCellMouseDidExited(self)
    }

    func renderColor(with foregroundColor: NSColor, isSmallEntry: Bool) {
        foregroundBox.fillColor = foregroundColor
        backgroundBox?.fillColor = foregroundColor.lighten(by: 0.1)
        backgroundBox?.borderColor = foregroundColor

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
