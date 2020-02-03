//
//  TimelineResizeHoverController.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 2/3/20.
//  Copyright © 2020 Alari. All rights reserved.
//

import Foundation

final class TimelineResizeHoverController: NSViewController {

    // MARK: OUTLET

    @IBOutlet weak var timeLabel: NSTextField!
    @IBOutlet weak var durationLabel: NSTextField!


    // MARK: View Cycle

    override func viewDidLoad() {
        super.viewDidLoad()
    }

    // MARK: Public

    func updateLabels(with startTime: TimeInterval, endTime: TimeInterval) {
        let startTimeStr = TimelineDateFormatter.shared.convertToHours(Date(timeIntervalSince1970: startTime))
        let endTimeStr = TimelineDateFormatter.shared.convertToHours(Date(timeIntervalSince1970: endTime))
        timeLabel.stringValue = "\(startTimeStr) - \(endTimeStr)"
        durationLabel.stringValue = DesktopLibraryBridge.shared().formatDurationTimestampt(abs(endTime - startTime)) ?? ""
    }
}
