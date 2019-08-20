//
//  TimelineActivityHoverController.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 8/20/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

final class TimelineActivityHoverController: NSViewController {

    // MARK: OUTLET

    @IBOutlet weak var timeLbl: NSTextField!
    @IBOutlet weak var eventStackView: NSStackView!

    // MARK: View

    override func viewDidLoad() {
        super.viewDidLoad()

    }

    // MARK: Public

    func render(_ activity: TimelineActivity) {
        timeLbl.stringValue = activity.startTimeStr
    }
}
