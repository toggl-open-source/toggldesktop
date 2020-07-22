//
//  TimelineResizeHoverController.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 2/3/20.
//  Copyright © 2020 Alari. All rights reserved.
//

import Foundation

/// Responsible for presenting the Resize Info Content on the Timeline Time Entry
/// It's a root controller of the NSPopover
/// This view should be shown during the resize session
final class TimelineResizeHoverController: NSViewController {

    // MARK: OUTLET

    @IBOutlet weak var timeLabel: NSTextField!
    @IBOutlet weak var durationLabel: NSTextField!

    // MARK: View Cycle

    override func viewDidLoad() {
        super.viewDidLoad()
    }

    // MARK: Public

    /// Update resize infomation
    /// - Parameters:
    ///   - startTime: The start time of the Entry
    ///   - endTime: The End time of the Entry
    func updateLabels(with startTime: TimeInterval, endTime: TimeInterval) {
        let startTimeStr = TimelineDateFormatter.shared.convertToHours(Date(timeIntervalSince1970: startTime))
        let endTimeStr = TimelineDateFormatter.shared.convertToHours(Date(timeIntervalSince1970: endTime))
        timeLabel.stringValue = "\(startTimeStr) - \(endTimeStr)"
        durationLabel.stringValue = DesktopLibraryBridge.shared().formatDurationTimestampt(abs(endTime - startTime)) ?? ""
    }
}
