//
//  TimelineTimeEntryMenu.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 7/2/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

protocol TimelineTimeEntryMenuDelegate: class {

    func shouldChangeFirstEntryStopTime()
    func shouldChangeLastEntryStartTime()
}

final class TimelineTimeEntryMenu: NSMenu {

    weak var menuDelegate: TimelineTimeEntryMenuDelegate?

    init() {
        super.init(title: "Menu")
        initSubmenu()
    }

    required init(coder decoder: NSCoder) {
        super.init(coder: decoder)
    }
}

// MARK: Private

extension TimelineTimeEntryMenu {

    fileprivate func initSubmenu() {
        let firstMenuItem = NSMenuItem(title: "Change first entry stop time", action: #selector(self.changeFirstEntryStopTimeOnTap), keyEquivalent: "")
        firstMenuItem.target = self
        let secondMenuImte = NSMenuItem(title: "Change last entry start time", action: #selector(self.changeLastEntryStartTimeOnTap), keyEquivalent: "")
        secondMenuImte.target = self

        addItem(firstMenuItem)
        addItem(secondMenuImte)
    }

    @objc private func changeFirstEntryStopTimeOnTap() {
        menuDelegate?.shouldChangeFirstEntryStopTime()
    }

    @objc private func changeLastEntryStartTimeOnTap() {
        menuDelegate?.shouldChangeLastEntryStartTime()
    }
}
