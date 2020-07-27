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

    @IBOutlet weak var startButton: NSHoverButton!
    @IBOutlet weak var durationLabel: NSTextField!
    @IBOutlet weak var descriptionTextField: BetterFocusAutoCompleteInput!
    @IBOutlet weak var descriptionContainerBox: TimerContainerBox!

    override func viewDidLoad() {
        super.viewDidLoad()

        descriptionTextField.displayMode = .fullscreen
        descriptionTextField.responderDelegate = descriptionContainerBox

        setupNotificationObservers()

        return

        let descriptionField = generateDescriptionField()
        let actionButton = generateStartStopButton()

        let mainStackView = NSStackView(views: [descriptionField, actionButton])
        mainStackView.orientation = .horizontal
//        mainStackView.distribution = .fill

        let projectButton = NSButton(frame: NSRect.zero)
        projectButton.stringValue = "Project"

        let additionsStackView = NSStackView(views: [projectButton])
        additionsStackView.orientation = .horizontal
//        additionsStackView.distribution = .fill

        let containerStackView = NSStackView(views: [mainStackView, additionsStackView])
        containerStackView.orientation = .vertical
        containerStackView.alignment = .leading

        view.addSubview(containerStackView)

        containerStackView.translatesAutoresizingMaskIntoConstraints = false
        let offset: CGFloat = 10
        NSLayoutConstraint.activate([
            containerStackView.topAnchor.constraint(equalTo: view.topAnchor, constant: offset),
            containerStackView.leadingAnchor.constraint(equalTo: view.leadingAnchor, constant: offset),
            containerStackView.bottomAnchor.constraint(equalTo: view.bottomAnchor, constant: offset),
            containerStackView.trailingAnchor.constraint(equalTo: view.trailingAnchor, constant: -offset)
        ])
    }

    deinit {
        cancelNotificationObservers()
    }

    @IBAction func starButtonClicked(_ sender: Any) {
        if timeEntry.isRunning() {
            NotificationCenter.default.post(name: NSNotification.Name(rawValue: kCommandStop), object: nil, userInfo: nil)
        } else {
            timeEntry.description = descriptionTextField.stringValue
            startButton.window?.makeFirstResponder(nil)
            NotificationCenter.default.post(name: NSNotification.Name(rawValue: kCommandNew), object: timeEntry, userInfo: nil)
        }
    }

    @IBAction func descriptionFieldChanged(_ sender: Any) {
        timeEntry.description = descriptionTextField.stringValue
    }

//    func control(_ control: NSControl, textView: NSTextView, doCommandBy commandSelector: Selector) -> Bool {
//    }

    // MARK: - Notifications handling

    func setupNotificationObservers() {
        NotificationCenter.default.addObserver(forName: NSNotification.Name(kDisplayTimerState), object: nil, queue: .main) { [weak self] notification in
            self?.updateTimerState(with: notification.object as? TimeEntryViewItem)
        }

        NotificationCenter.default.addObserver(forName: NSNotification.Name(kFocusTimer), object: nil, queue: .main) { [weak self] notification in
            self?.focusTimer()
        }

        NotificationCenter.default.addObserver(forName: NSNotification.Name(kCommandStop), object: nil, queue: .main) { [weak self] notification in
            self?.stop()
        }

        NotificationCenter.default.addObserver(forName: NSNotification.Name(kStartTimer), object: nil, queue: .main) { [weak self] notification in
            guard let self = self else { return }
            self.starButtonClicked(self)
        }
    }

    func cancelNotificationObservers() {
        //
    }

    func updateTimerState(with timeEntry: TimeEntryViewItem?) {
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

    func focusTimer() {
        descriptionTextField.window?.makeFirstResponder(descriptionTextField)
    }

    func stop() {
        startButton.toolTip = "Start"
        startButton.state = .off
        if descriptionTextField.currentEditor() == nil {
            descriptionTextField.stringValue = ""
        }
        durationLabel.stringValue = ""
        timeEntry = TimeEntryViewItem()

        focusTimer()
    }


    // MARK: - Generate UI

    // TODO: remove hardcoded values

    private func generateDescriptionField() -> NSView {
        let containerBox = TimerContainerBox(frame: NSRect.zero)
        containerBox.titlePosition = .noTitle
        containerBox.boxType = .custom
        containerBox.borderType = .grooveBorder
        containerBox.borderWidth = 1
        containerBox.cornerRadius = 15
        if #available(OSX 10.13, *) {
            containerBox.borderColor = NSColor(named: "lighter-grey-color")!
            containerBox.fillColor = NSColor(named: "preference-box-background-color")!
        } else {
            containerBox.borderColor = ConvertHexColor.hexCode(toNSColor: "#ACACAC")
            containerBox.fillColor = NSColor(deviceRed: 177.0/255.0, green: 177.0/255.0, blue: 177.0/255.0, alpha: 0.07)
        }

        NSLayoutConstraint.activate([
            containerBox.heightAnchor.constraint(equalToConstant: 30)
        ])

        let field = BetterFocusAutoCompleteInput(frame: NSRect.zero)
        let fieldCell = VerticallyCenteredTextFieldCell()
        fieldCell.focusRingCornerRadius = 12
        fieldCell.placeholderString = "What are you working on?"
        field.cell = fieldCell

        field.isBordered = false
        field.placeholderString = "What are you working on?"

        field.translatesAutoresizingMaskIntoConstraints = false
        containerBox.addSubview(field)
//        field.edgesToSuperView()

        field.responderDelegate = containerBox

        return containerBox
    }

    private func generateStartStopButton() -> NSButton {
        let button = NSHoverButton(frame: NSRect.zero)
        button.image = #imageLiteral(resourceName: "start-timer-icon")
        button.alternateImage = #imageLiteral(resourceName: "stop-timer-button")
        button.setButtonType(.switch)
        button.isBordered = false
        button.title = ""
        return button
    }

}

// MARK: - NSTextFieldDelegate

extension TimerViewController: NSTextFieldDelegate {
    //
}
