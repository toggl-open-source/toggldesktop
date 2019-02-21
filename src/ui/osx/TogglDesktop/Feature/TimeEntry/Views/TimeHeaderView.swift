//
//  TimeHeaderView.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 2/20/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

final class TimeHeaderView: NSView {

    // MARK: Varibles

    @IBOutlet weak var dateLbl: NSTextField!
    @IBOutlet weak var totalDurationLbl: NSTextField!

    // MARK: Public
    
    func config(_ header: TimeEntryHeader) {
        dateLbl.stringValue = header.date
        totalDurationLbl.stringValue = header.totalTime
    }
}
