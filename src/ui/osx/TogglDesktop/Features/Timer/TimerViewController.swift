//
//  TimerViewController.swift
//  TogglDesktop
//
//  Created by Andrew Nester on 27.07.2020.
//  Copyright © 2020 Toggl. All rights reserved.
//

import Cocoa

class TimerViewController: NSViewController {

    var timeEntry = TimeEntryViewItem()

    private var notificationObservers: [AnyObject] = []

    // MARK: - Outlets

    @IBOutlet weak var startButton: NSHoverButton!
    @IBOutlet weak var durationLabel: NSTextField!
    @IBOutlet weak var descriptionTextField: BetterFocusAutoCompleteInput!
    @IBOutlet weak var descriptionContainerBox: TimerContainerBox!
    @IBOutlet weak var projectButton: SelectableButton!
    @IBOutlet weak var tagsButton: SelectableButton!
    @IBOutlet weak var billableButton: SelectableButton!

    // MARK: - Lifecycle

    override func viewDidLoad() {
        super.viewDidLoad()

        descriptionTextField.displayMode = .fullscreen
        descriptionTextField.responderDelegate = descriptionContainerBox

        setupNotificationObservers()

        configureAppearance()
    }

    deinit {
        cancelNotificationObservers()
    }

    // MARK: - Actions

    @IBAction func starButtonClicked(_ sender: Any) {
        if timeEntry.isRunning() {
            NotificationCenter.default.post(name: NSNotification.Name(rawValue: kCommandStop), object: nil, userInfo: nil)
        } else {
            timeEntry.description = descriptionTextField.stringValue
            startButton.window?.makeFirstResponder(nil)
            NotificationCenter.default.post(name: NSNotification.Name(rawValue: kCommandNew), object: timeEntry, userInfo: nil)
        }
    }

    @IBAction func projectButtonClicked(_ sender: Any) {
    }

    @IBAction func tagsButtonClicked(_ sender: Any) {
        NSLog("Tag mouse clicked")

        if tagsButton.controlState == .active {
            if tagsButton.isSelected == true {
                tagsButton.isSelected = false
            } else {
                tagsButton.isSelected = true
                tagsButton.controlState = .normal
            }
        }
    }

    @IBAction func billableButtonClicked(_ sender: Any) {
    }

    @IBAction func descriptionFieldChanged(_ sender: Any) {
        timeEntry.description = descriptionTextField.stringValue
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
            guard let self = self else { return }
            self.starButtonClicked(self)
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
            if descriptionTextField.currentEditor() == nil {
                entry = TimeEntryViewItem()
            } else {
                return
            }
        }
        self.timeEntry = entry

        if entry.isRunning() {
            startButton.toolTip = "Stop"
            startButton.state = .on

            if let description = entry.descriptionName, !description.isEmpty {
                descriptionTextField.stringValue = description
                descriptionTextField.toolTip = description
            } else {
                // TODO: test if we need this, maybe placeholder is enough
//                descriptionTextField.stringValue = ""
//                descriptionTextField.toolTip = "(no description)"
            }

            if let durationString = entry.startTimeString {
                durationLabel.toolTip = "Started: \(durationString)"
            }

            // TODO: update buttons etc.

            // TODO: don't know if needed
            // Switch to timer mode in setting
//            toggl_set_settings_manual_mode(ctx, false);
        } else {
            //
        }

        durationLabel.stringValue = entry.duration ?? ""

        if #available(macOS 10.12.2, *) {
            if entry.isRunning() {
                TouchBarService.shared.updateRunningItem(entry)
            }
        }
    }

    private func focusTimer() {
        descriptionTextField.window?.makeFirstResponder(descriptionTextField)
    }

    private func stop() {
        startButton.toolTip = "Start"
        startButton.state = .off
        if descriptionTextField.currentEditor() == nil {
            descriptionTextField.stringValue = ""
        }
        durationLabel.stringValue = ""
        timeEntry = TimeEntryViewItem()

        focusTimer()
    }

    // MARK: - UI

    private func configureAppearance() {
        [projectButton, tagsButton, billableButton]
            .compactMap { $0 }
            .forEach {
                $0.cornerRadius = $0.bounds.height / 2
                $0.selectedBackgroundColor = NSColor.togglGreen
//                $0.tintColor = NSColor.togglLighterGrey
//                $0.hoverTintColor = NSColor.togglGreyText
        }
    }
}

// MARK: - NSTextFieldDelegate

extension TimerViewController: NSTextFieldDelegate {
    //
}
