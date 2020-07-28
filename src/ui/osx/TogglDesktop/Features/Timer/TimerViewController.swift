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

        configureAppearance()

        setupBindings()
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
        }

        viewModel.onDescriptionChanged = { [unowned self] description in
            if self.descriptionTextField.currentEditor() == nil {
                self.descriptionTextField.stringValue = description
            }
        }

        viewModel.onDurationChanged = { [unowned self] duration in
            self.durationLabel.stringValue = duration
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
        viewModel.description = descriptionTextField.stringValue
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
