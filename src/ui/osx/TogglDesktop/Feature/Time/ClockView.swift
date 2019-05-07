//
//  ClockView.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 5/6/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

class ClockView: NSView {

    enum DisplayMode {
        case minute
        case hour12
        case hour24
    }

    // MARK: OUTLET

    @IBOutlet weak var minuteContainerView: NSView!

    // MARK: Variables

    private var displayMode: DisplayMode = . minute {
        didSet {
            layoutClock()
        }
    }

    // MARK: View Cycle

    override func awakeFromNib() {
        super.awakeFromNib()
    }

    @IBAction func minuteBtnOnTap(_ sender: NSButton) {

    }

    @IBAction func hourBtnOnTap(_ sender: NSButton) {

    }

    func config(with timeEntry: TimeEntryViewItem, displayMode: DisplayMode) {

    }
}

// MARK: Private

extension ClockView {

    fileprivate func initCommon() {

    }

    fileprivate func layoutClock() {

    }
}
