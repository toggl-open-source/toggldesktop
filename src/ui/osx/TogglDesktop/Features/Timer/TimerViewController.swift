//
//  TimerViewController.swift
//  TogglDesktop
//
//  Created by Andrew Nester on 27.07.2020.
//  Copyright © 2020 Toggl. All rights reserved.
//

import Cocoa

class TimerViewController: NSViewController {

    let viewModel = TimerViewModel()

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

        viewModel.setAutocompleteInput(descriptionTextField)

        configureAppearance()

        setupBindings()
    }

    override func viewDidAppear() {
        super.viewDidAppear()

        let viewFrameInWindowCoords = view.convert(view.bounds, to: nil)
        descriptionTextField.setPos(Int32(viewFrameInWindowCoords.origin.y))
    }

    private func setupBindings() {
        viewModel.onDescriptionFocusChanged = { [unowned self] shouldFocus in
            if shouldFocus {
                self.descriptionTextField.window?.makeFirstResponder(self.descriptionTextField)
            } else {
                self.descriptionTextField.window?.makeFirstResponder(nil)
            }
        }

        viewModel.onIsRunning = { [unowned self] isRunning in
            if isRunning {
                self.startButton.toolTip = "Stop"
                self.startButton.state = .on
            } else {
                self.startButton.toolTip = "Start"
                self.startButton.state = .off
            }
            NotificationCenter.default.post(name: NSNotification.Name(rawValue: kStartButtonStateChange),
                                            object: NSNumber(value: self.startButton.state.rawValue))
        }

        viewModel.onDescriptionChanged = { [unowned self] description in
            if self.descriptionTextField.stringValue != description {
                self.descriptionTextField.stringValue = description
            }
        }

        viewModel.onDurationChanged = { [unowned self] duration in
            self.durationLabel.stringValue = duration
        }

        viewModel.onTagSelected = { [unowned self] isSelected in
            self.tagsButton.isSelected = isSelected
        }

        viewModel.onProjectSelected = { [unowned self] project in
            if let project = project {
                self.projectButton.selectedBackgroundColor = project.color.withAlphaComponent(0.3)
                self.projectButton.attributedTitle = project.attributedTitle
                self.projectButton.image = nil
                self.projectButton.isSelected = true
            } else {
                self.projectButton.title = ""
                self.projectButton.image = NSImage(named: "project-button")
                self.projectButton.isSelected = false
            }
        }

        viewModel.onBillableChanged = { [unowned self] billable in
            self.billableButton.isEnabled = billable != .notAvailable
            self.billableButton.isSelected = billable == .on
        }

        viewModel.onTouchBarUpdateRunningItem = { entry in
            if #available(macOS 10.12.2, *) {
                TouchBarService.shared.updateRunningItem(entry)
            }
        }
    }

    // MARK: - Actions

    @IBAction func starButtonClicked(_ sender: Any) {
        viewModel.start()
    }

    @IBAction func projectButtonClicked(_ sender: Any) {
    }

    @IBAction func tagsButtonClicked(_ sender: Any) {
    }

    @IBAction func billableButtonClicked(_ sender: Any) {
    }

    @IBAction func descriptionFieldChanged(_ sender: Any) {
        viewModel.entryDescription = descriptionTextField.stringValue
    }

    func controlTextDidChange(_ obj: Notification) {
        if let descriptionField = obj.object as? AutoCompleteInput {
            viewModel.entryDescription = descriptionField.stringValue
        }
    }

    func control(_ control: NSControl, textView: NSTextView, doCommandBy commandSelector: Selector) -> Bool {
        var isHandled = false

        if commandSelector == #selector(moveDown(_:)) {
            descriptionTextField.autocompleteTableView.nextItem()
        } else if commandSelector == #selector(moveUp(_:)) {
            descriptionTextField.autocompleteTableView.previousItem()
        } else if commandSelector == #selector(insertTab(_:)) {
            // set data according to selected item
            let lastSelectedIndex = descriptionTextField.autocompleteTableView.lastSelected
            if lastSelectedIndex >= 0 {
                let success = viewModel.selectAutocompleteItem(at: lastSelectedIndex)
                if !success {
                    return false
                }
            }
        } else if commandSelector == #selector(insertNewline(_:)) {
            // avoid firing default Enter actions
            isHandled = true
            let lastSelectedIndex = descriptionTextField.autocompleteTableView.lastSelected
            if lastSelectedIndex >= 0 {
                let success = viewModel.selectAutocompleteItem(at: lastSelectedIndex)
                if !success {
                    return isHandled
                }
            }
            viewModel.start()
        }

        return isHandled
    }

    // MARK: - UI

    private func configureAppearance() {
        [projectButton, tagsButton, billableButton]
            .compactMap { $0 }
            .forEach {
                $0.cornerRadius = $0.bounds.height / 2
                $0.selectedBackgroundColor = NSColor.togglGreen
        }
    }
}

// MARK: - NSTextFieldDelegate

extension TimerViewController: NSTextFieldDelegate {
    //
}
