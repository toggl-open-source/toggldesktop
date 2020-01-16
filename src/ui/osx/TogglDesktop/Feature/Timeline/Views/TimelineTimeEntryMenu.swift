//
//  TimelineTimeEntryMenu.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 7/2/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

protocol TimelineTimeEntryMenuDelegate: class {

    func timelineMenuContinue(_ timeEntry: TimelineTimeEntry)
    func timelineMenuStartEntry(_ timeEntry: TimelineTimeEntry)
    func timelineMenuDelete(_ timeEntry: TimelineTimeEntry)
    func timelineMenuChangeFirstEntryStopTime(_ timeEntry: TimelineTimeEntry)
    func timelineMenuChangeLastEntryStartTime(_ timeEntry: TimelineTimeEntry)
}

final class TimelineTimeEntryMenu: NSMenu {

    // MARK: Variables

    weak var menuDelegate: TimelineTimeEntryMenuDelegate?
    var timeEntry: TimelineTimeEntry? { didSet { updateMenuTitle() }}
    private var conflictChangeFirstMenu: NSMenuItem!
    private var conflictChangeLastMenu: NSMenuItem!
    private var startNewMenu: NSMenuItem!
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
        let deleteMenu = NSMenuItem(title: "Delete", action: #selector(self.deleteEntryOnTap), keyEquivalent: "")
        startNewMenu = NSMenuItem(title: "Start entry from the end of this entry", action: #selector(self.startEntryOnTap), keyEquivalent: "")
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

    private func updateMenuTitle() {
        guard let isToday = timeEntry?.isToday() else { return }
        startNewMenu.title = isToday ? "Start entry from the end of this entry" : "Create entry from the end of this entry"
    }

    @objc private func continueMenuOnTap() {
        guard let timeEntry = timeEntry else { return }
        menuDelegate?.timelineMenuContinue(timeEntry)
    }

    @objc private func startEntryOnTap() {
        guard let timeEntry = timeEntry else { return }
        menuDelegate?.timelineMenuStartEntry(timeEntry)
    }

    @objc private func deleteEntryOnTap() {
        guard let timeEntry = timeEntry else { return }
        menuDelegate?.timelineMenuDelete(timeEntry)
    }

    @objc private func changeFirstEntryStopTimeOnTap() {
        guard let timeEntry = timeEntry else { return }
        menuDelegate?.timelineMenuChangeFirstEntryStopTime(timeEntry)
    }

    @objc private func changeLastEntryStartTimeOnTap() {
        guard let timeEntry = timeEntry else { return }
        menuDelegate?.timelineMenuChangeLastEntryStartTime(timeEntry)
    }
}
