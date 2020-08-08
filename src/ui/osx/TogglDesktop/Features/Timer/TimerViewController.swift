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

    private lazy var projectAutoCompleteWindow: AutoCompleteViewWindow = {
        return AutoCompleteViewWindow(view: projectAutoCompleteView)
    }()
    private var projectAutoCompleteView: AutoCompleteView = AutoCompleteView.xibView()
    private var projectAutocompleteDidResignObserver: Any?

    private lazy var tagsAutoCompleteWindow: AutoCompleteViewWindow = {
        return AutoCompleteViewWindow(view: tagsAutoCompleteView)
    }()
    private var tagsAutoCompleteView: AutoCompleteView = AutoCompleteView.xibView()
    private var tagsAutocompleteDidResignObserver: Any?

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

        viewModel.isEditingDescription = { [weak self] in
            self?.descriptionTextField.currentEditor() != nil
        }
    }

    override func viewDidAppear() {
        super.viewDidAppear()

        let viewFrameInWindowCoords = view.convert(view.bounds, to: nil)
        descriptionTextField.setPos(Int32(viewFrameInWindowCoords.origin.y))

        // !!!: we're passing views into view model - refactor this someday
        // this is needed because current Autocomplete functionality
        // is tightly coupled with text input views
        viewModel.setDescriptionAutoCompleteInput(descriptionTextField)

        viewModel.projectDataSource.setup(with: projectAutoCompleteView)
        viewModel.tagsDataSource.setup(with: tagsAutoCompleteView)

        viewModel.prepareData()

        configureHideAutoCompleteWhenLostFocus()
    }

    override func viewDidDisappear() {
        super.viewDidDisappear()

        [projectAutocompleteDidResignObserver, tagsAutocompleteDidResignObserver]
            .compactMap { $0 }
            .forEach { NotificationCenter.default.removeObserver($0) }
    }

    private func setupBindings() {
        viewModel.onDescriptionFocusChanged = { [unowned self] shouldFocus in
            if shouldFocus {
                self.focusDescriptionField()
            } else {
                self.descriptionTextField.window?.makeFirstResponder(nil)
            }
        }

        viewModel.onIsRunning = { [unowned self] isRunning in
            self.startButton.toolTip = isRunning ? "Stop" : "Start"
            self.startButton.state = isRunning ? .on : .off
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
            self.closeProjectAutoComplete()

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
            self.billableButton.isEnabled = billable != .unavailable
            self.billableButton.isSelected = billable == .on
        }

        viewModel.onTouchBarUpdateRunningItem = { entry in
            if #available(macOS 10.12.2, *) {
                TouchBarService.shared.updateRunningItem(entry)
            }
        }
    }

    @objc
    func focusDescriptionField() {
        guard let window = view.window, window.firstResponder != descriptionTextField.currentEditor() else {
            return
        }
        window.makeFirstResponder(descriptionTextField)
    }

    // TODO: consider removing this after refactoring TimerEditViewController
    @objc
    func triggerStartStopAction() {
        viewModel.startStopAction()
    }

    // MARK: - Actions

    @IBAction func starButtonClicked(_ sender: Any) {
        viewModel.startStopAction()
    }

    @IBAction func projectButtonClicked(_ sender: Any) {
        if projectAutoCompleteWindow.isVisible {
            closeProjectAutoComplete()
        } else {
            presentProjectAutoComplete()
        }
    }

    @IBAction func tagsButtonClicked(_ sender: Any) {
        if tagsAutoCompleteWindow.isVisible {
            closeTagsAutoComplete()
        } else {
            presentTagsAutoComplete()
        }
    }

    @IBAction func billableButtonClicked(_ sender: Any) {
        viewModel.setBillable(!billableButton.isSelected)
    }

    func controlTextDidChange(_ obj: Notification) {
        if let textField = obj.object as? AutoCompleteInput, textField == descriptionTextField {
            viewModel.setDescription(textField.stringValue)
        }
    }

    func controlTextDidEndEditing(_ obj: Notification) {
        viewModel.descriptionDidEndEditing()
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
                let success = viewModel.selectDescriptionAutoCompleteItem(at: lastSelectedIndex)
                if !success {
                    return false
                }
            }
        } else if commandSelector == #selector(insertNewline(_:)) {
            // avoid firing default Enter actions
            isHandled = true
            let lastSelectedIndex = descriptionTextField.autocompleteTableView.lastSelected
            if lastSelectedIndex >= 0 {
                let success = viewModel.selectDescriptionAutoCompleteItem(at: lastSelectedIndex)
                if !success {
                    return isHandled
                }
            }
            if !viewModel.isRunning {
                viewModel.startStopAction()
            } else {
                view.window?.makeFirstResponder(nil)
                descriptionTextField.resetTable()
            }
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
        billableButton.isActiveOnClick = false
    }

    // MARK: - Autocomplete/Dropdown

    private func presentProjectAutoComplete() {
        let fromPoint = NSPoint(x: projectButton.frame.minX, y: projectButton.frame.maxY)
        presentAutoComplete(window: projectAutoCompleteWindow, withContentView: projectAutoCompleteView, fromPoint: fromPoint)
        viewModel.projectDataSource.sizeToFit()
    }

    private func presentTagsAutoComplete() {
        let fromPoint = NSPoint(x: tagsButton.frame.minX, y: tagsButton.frame.maxY)
        presentAutoComplete(window: tagsAutoCompleteWindow, withContentView: tagsAutoCompleteView, fromPoint: fromPoint)
        viewModel.tagsDataSource.sizeToFit()
    }

    private func presentAutoComplete(window: NSWindow, withContentView contentView: NSView, fromPoint: NSPoint) {
        window.contentView = contentView
        window.setContentSize(contentView.frame.size)

        if !window.isVisible {
            let windowRect = autoCompleteWindowRect(fromPoint: fromPoint)
            window.setFrame(windowRect, display: false)
            window.setFrameTopLeftPoint(windowRect.origin)

            if view.window != window {
                view.window?.addChildWindow(window, ordered: .above)
            }
            window.makeKeyAndOrderFront(nil)
            window.makeFirstResponder(contentView)
        }
    }

    private func autoCompleteWindowRect(fromPoint: NSPoint) -> NSRect {
        let padding: CGFloat = 6
        // passing random height value as it should be updated with to its content size later
        let initialHeight: CGFloat = 100
        let windowSize = NSSize(width: view.frame.width - padding * 2, height: initialHeight)
        var rect = NSRect(origin: .zero, size: windowSize)

        let windowPoint = projectButton.convert(fromPoint, to: nil)
        var screenPoint = CGPoint.zero
        if #available(OSX 10.12, *) {
            screenPoint = view.window?.convertPoint(toScreen: windowPoint) ?? .zero
        } else {
            screenPoint = view.window?.convertToScreen(NSRect(origin: windowPoint, size: .zero)).origin ?? .zero
        }
        rect.origin = screenPoint

        return rect
    }

    private func configureHideAutoCompleteWhenLostFocus() {
        projectAutocompleteDidResignObserver = NotificationCenter.default.addObserver(
            forName: NSWindow.didResignKeyNotification,
            object: projectAutoCompleteWindow,
            queue: .main
        ) { [unowned self] _ in
            if self.projectAutoCompleteWindow.isVisible {
                NSLog("<<< Lost focus -> closing project autocomplete")
                self.closeProjectAutoComplete()
            }
        }

        tagsAutocompleteDidResignObserver = NotificationCenter.default.addObserver(
            forName: NSWindow.didResignKeyNotification,
            object: tagsAutoCompleteWindow,
            queue: .main
        ) { [unowned self] _ in
            if self.tagsAutoCompleteWindow.isVisible {
                self.closeTagsAutoComplete()
            }
        }
    }

    private func closeProjectAutoComplete() {
        projectAutoCompleteWindow.cancel()
        projectButton.controlState = .normal
    }

    private func closeTagsAutoComplete() {
        tagsAutoCompleteWindow.cancel()
        tagsButton.controlState = .normal
    }
}

// MARK: - NSTextFieldDelegate

extension TimerViewController: NSTextFieldDelegate {
    //
}
