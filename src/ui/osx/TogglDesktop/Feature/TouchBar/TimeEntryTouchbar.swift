//
//  TimeEntryTouchbar.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 10/1/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

@available(OSX 10.12.2, *)
@objcMembers
final class TimeEntryTouchBar: NSObject {

    static let shared = TimeEntryTouchBar()

    // MARK: OUTLET

    private lazy var startButton: NSButton = {
        let btn = NSButton(title: "Start", target: self, action: #selector(self.startBtnOnTap(_:)))
        btn.title = "Start"
        btn.alternateTitle = "Stop"
        return btn
    }()

    // MARK: Variables

    lazy var touchBar = NSTouchBar()

    // MARK: Init

    override init() {
        super.init()
        initCommon()
        setup()
        initNotification()
    }
}

@available(OSX 10.12.2, *)
extension TimeEntryTouchBar {

    fileprivate func initCommon() {
        NSApplication.shared.isAutomaticCustomizeTouchBarMenuItemEnabled = true
    }

    fileprivate func setup() {
        touchBar.delegate = self
        touchBar.customizationIdentifier = .timeEntry
        touchBar.defaultItemIdentifiers = [.timeEntryItem, .flexibleSpace, .runningTimeEntry, .startStopItem]
    }

    fileprivate func initNotification() {
        NotificationCenter.default.addObserver(self,
                                               selector: #selector(self.stateButtonTimerBarChangeNotification(_:)),
                                               name: NSNotification.Name(kStartButtonStateChange),
                                               object: nil)
    }

    @objc private func stateButtonTimerBarChangeNotification(_ noti: Notification) {
        guard let state = noti.object as? NSButton.StateValue else {
            return
        }
        startButton.state = state
    }
}

// MARK: NSTouchBarDelegate

@available(OSX 10.12.2, *)
extension TimeEntryTouchBar: NSTouchBarDelegate {

    func touchBar(_ touchBar: NSTouchBar, makeItemForIdentifier identifier: NSTouchBarItem.Identifier) -> NSTouchBarItem? {
        switch identifier {
        case NSTouchBarItem.Identifier.timeEntryItem:
            let item = NSCustomTouchBarItem(identifier: identifier)
            let button = NSButton(title: "TimeEntry 1", target: nil, action: nil)
            item.view = button
            return item
        case NSTouchBarItem.Identifier.runningTimeEntry:
            let item = NSCustomTouchBarItem(identifier: identifier)
            let button = NSButton(title: "Running...", target: nil, action: nil)
            item.view = button
            return item
        case NSTouchBarItem.Identifier.startStopItem:
            let item = NSCustomTouchBarItem(identifier: identifier)
            item.view = startButton
            return item
        default:
            return nil
        }
    }
}

@available(OSX 10.12.2, *)
extension NSTouchBar.CustomizationIdentifier {
    static let timeEntry = NSTouchBar.CustomizationIdentifier("com.toggl.toggldesktop.timeentrytouchbar")
}

@available(OSX 10.12.2, *)
extension NSTouchBarItem.Identifier {

    static let timeEntryItem = NSTouchBarItem.Identifier("com.toggl.toggldesktop.timeentrytouchbar.timeentryitems")
    static let runningTimeEntry = NSTouchBarItem.Identifier("com.toggl.toggldesktop.timeentrytouchbar.runningtimeentry")
    static let startStopItem = NSTouchBarItem.Identifier("com.toggl.toggldesktop.timeentrytouchbar.startstopbutton")
}

// MARK: Private

@available(OSX 10.12.2, *)
extension TimeEntryTouchBar {

    @objc fileprivate func startBtnOnTap(_ sender: NSButton) {

    }
}
