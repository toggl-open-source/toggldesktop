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

    // MARK: Variables

    weak var menuDelegate: TimelineTimeEntryMenuDelegate?
    private var conflictChangeFirstMenu: NSMenuItem!
    private var conflictChangeLastMenu: NSMenuItem!
    var isOverlapMenu = false {
        didSet {
            conflictChangeFirstMenu.isEnabled = isOverlapMenu
            conflictChangeLastMenu.isEnabled = isOverlapMenu
        }
    }

    // MARK: Init

    init() {
        super.init(title: "Menu")
        initCommon()
        initSubmenu()
    }

    required init(coder decoder: NSCoder) {
        super.init(coder: decoder)
    }
}

// MARK: Private

extension TimelineTimeEntryMenu {

    private func initCommon() {
        autoenablesItems = false
    }

    private func initSubmenu() {
        let continueMenu = NSMenuItem(title: "Continue this entry", action: #selector(self.continueMenuOnTap), keyEquivalent: "")
        let startNewMenu = NSMenuItem(title: "Start entry from the end of this entry", action: #selector(self.startEntryOnTap), keyEquivalent: "")
        let deleteMenu = NSMenuItem(title: "Delete", action: #selector(self.deleteEntryOnTap), keyEquivalent: "")
        conflictChangeFirstMenu = NSMenuItem(title: "Change first entry stop time", action: #selector(self.changeFirstEntryStopTimeOnTap), keyEquivalent: "")
        conflictChangeLastMenu = NSMenuItem(title: "Change last entry start time", action: #selector(self.changeLastEntryStartTimeOnTap), keyEquivalent: "")

        // Default items
        let menus: [NSMenuItem] = [continueMenu,
                                   startNewMenu,
                                   deleteMenu,
                                   NSMenuItem.separator(),
                                   conflictChangeFirstMenu,
                                   conflictChangeLastMenu]
        menus.forEach { item in
            item.target = self
            addItem(item)
        }
    }

    @objc private func continueMenuOnTap() {
    }

    @objc private func startEntryOnTap() {
    }

    @objc private func deleteEntryOnTap() {
    }

    @objc private func changeFirstEntryStopTimeOnTap() {
        menuDelegate?.shouldChangeFirstEntryStopTime()
    }

    @objc private func changeLastEntryStartTimeOnTap() {
        menuDelegate?.shouldChangeLastEntryStartTime()
    }
}
