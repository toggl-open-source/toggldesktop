//
//  TimerViewModel.swift
//  TogglDesktop
//
//  Created by Andrew Nester on 28.07.2020.
//  Copyright © 2020 Toggl. All rights reserved.
//

import Foundation

final class TimerViewModel {

    var description: String = "" {
        didSet {
            timeEntry.description = description
            onDescriptionChanged?(description)
        }
    }

    private(set) var durationString: String = "" {
        didSet {
            onDurationChanged?(durationString)
        }
    }

    private(set) var isRunning: Bool = false {
        didSet {
            onIsRunning?(isRunning)
        }
    }

    var onDescriptionFocusChanged: ((Bool) -> Void)?
    var onIsRunning: ((Bool) -> Void)?
    var onDescriptionChanged: ((String) -> Void)?
    var onDurationChanged: ((String) -> Void)?
    var onTouchBarUpdateRunningItem: ((TimeEntryViewItem) -> Void)?

    private var timeEntry = TimeEntryViewItem()
    private var notificationObservers: [AnyObject] = []

    init() {
        setupNotificationObservers()
    }

    deinit {
        cancelNotificationObservers()
    }

    func start() {
        if timeEntry.isRunning() {
            NotificationCenter.default.post(name: NSNotification.Name(rawValue: kCommandStop), object: nil, userInfo: nil)
        } else {
            timeEntry.description = description
            NotificationCenter.default.post(name: NSNotification.Name(rawValue: kCommandNew), object: timeEntry, userInfo: nil)
            onDescriptionFocusChanged?(false)
        }
    }

    // MARK: - Notifications handling

    private func setupNotificationObservers() {
        let displayTimerStateObserver = NotificationCenter.default.addObserver(forName: NSNotification.Name(kDisplayTimerState),
                                                                               object: nil,
                                                                               queue: .main) { [weak self] notification in
            self?.updateTimerState(with: notification.object as? TimeEntryViewItem)
        }

        let focusTimerObserver = NotificationCenter.default.addObserver(forName: NSNotification.Name(kFocusTimer),
                                                                        object: nil,
                                                                        queue: .main) { [weak self] _ in
            self?.focusTimer()
        }

        let commandStopObserver = NotificationCenter.default.addObserver(forName: NSNotification.Name(kCommandStop),
                                                                         object: nil,
                                                                         queue: .main) { [weak self] _ in
            self?.stop()
        }

        let startTimerObserver = NotificationCenter.default.addObserver(forName: NSNotification.Name(kStartTimer),
                                                                        object: nil,
                                                                        queue: .main) { [weak self] _ in
            self?.start()
        }

        notificationObservers = [displayTimerStateObserver, focusTimerObserver, commandStopObserver, startTimerObserver]
    }

    private func cancelNotificationObservers() {
        notificationObservers.forEach {
            NotificationCenter.default.removeObserver($0)
        }
    }

    private func updateTimerState(with timeEntry: TimeEntryViewItem?) {
        let entry: TimeEntryViewItem
        if let timeEntry = timeEntry {
            entry = timeEntry
        } else {
            entry = TimeEntryViewItem()
        }
        self.timeEntry = entry

        isRunning = entry.isRunning()

        if let description = entry.descriptionName, !description.isEmpty {
            self.description = description
//            descriptionTextField.toolTip = description
        }

        durationString = entry.duration ?? ""

//        if let durationString = entry.startTimeString {
//            durationLabel.toolTip = "Started: \(durationString)"
//        }

        // TODO: don't know if needed
        // Switch to timer mode in setting
//        toggl_set_settings_manual_mode(ctx, false);

        if entry.isRunning() {
            onTouchBarUpdateRunningItem?(entry)
        }
    }

    private func focusTimer() {
        onDescriptionFocusChanged?(true)
    }

    private func stop() {
        isRunning = false
        description = ""
        durationString = ""
        focusTimer()

        timeEntry = TimeEntryViewItem()
    }
}
