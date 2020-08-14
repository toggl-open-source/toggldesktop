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
    private var projectAutocompleteResignTime: TimeInterval = 0

    private lazy var projectCreationView: ProjectCreationView = {
        let view = ProjectCreationView.xibView() as ProjectCreationView
        view.delegate = self
        return view
    }()

    private lazy var tagsAutoCompleteWindow: AutoCompleteViewWindow = {
        return AutoCompleteViewWindow(view: tagsAutoCompleteView)
    }()
    private var tagsAutoCompleteView: AutoCompleteView = AutoCompleteView.xibView()
    private var tagsAutocompleteDidResignObserver: Any?
    private var tagsAutocompleteResignTime: TimeInterval = 0

    // MARK: - Outlets

    @IBOutlet weak var startButton: NSHoverButton!
    @IBOutlet weak var descriptionTextField: BetterFocusAutoCompleteInput!
    @IBOutlet weak var descriptionContainerBox: TimerContainerBox!
    @IBOutlet weak var durationContainerBox: TimerContainerBox!
    @IBOutlet weak var durationTextField: ResponderTextField!
    @IBOutlet weak var projectButton: SelectableButton!
    @IBOutlet weak var tagsButton: SelectableButton!
    @IBOutlet weak var billableButton: SelectableButton!

    // MARK: - Lifecycle

    override func viewDidLoad() {
        super.viewDidLoad()

        descriptionTextField.displayMode = .fullscreen
        descriptionTextField.responderDelegate = descriptionContainerBox

        durationTextField.responderDelegate = durationContainerBox

        projectAutoCompleteView.delegate = self
        tagsAutoCompleteView.delegate = self

        configureAppearance()
        setupProjectButtonContextMenu()
        setupKeyViewLoop()

        setupBindings()

        viewModel.isEditingDescription = { [weak self] in
            self?.descriptionTextField.currentEditor() != nil
        }

        viewModel.isEditingDuration = { [weak self] in
            return self?.durationTextField.currentEditor() != nil
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
            if self.durationTextField.stringValue != duration {
                self.durationTextField.stringValue = duration
            }
        }

        viewModel.onTagSelected = { [unowned self] isSelected in
            self.tagsButton.isSelected = isSelected
        }

        viewModel.onProjectUpdated = { [unowned self] project in
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
            self.setupProjectButtonContextMenu()
        }

        viewModel.onProjectSelected = { [unowned self] project in
            self.closeProjectAutoComplete()
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
        view.window?.makeFirstResponder(nil)
        viewModel.startStopAction()
    }

    @IBAction func projectButtonClicked(_ sender: Any) {
        // Small hack:
        // It is user friendly to close the dropdown if user click outside of it
        // and also if user click on the same button that opens this dropdown.
        // To prevent reopening the dropdown after user clicks on tagsButton
        // we don't show the dropdown if it was closed just half a second ago.
        let wasNotClosedJustNow = (Date().timeIntervalSince1970 - projectAutocompleteResignTime) > 0.5

        if wasNotClosedJustNow {
            presentProjectAutoComplete()
        } else {
            // returning state back to normal if click is not handled
            projectButton.controlState = .normal
        }
    }

    @IBAction func tagsButtonClicked(_ sender: Any) {
        // Small hack:
        // It is user friendly to close the dropdown if user click outside of it
        // and also if user click on the same button that opens this dropdown.
        // To prevent reopening the dropdown after user clicks on tagsButton
        // we don't show the dropdown if it was closed just half a second ago.
        let wasNotClosedJustNow = (Date().timeIntervalSince1970 - tagsAutocompleteResignTime) > 0.5

        if wasNotClosedJustNow {
            presentTagsAutoComplete()
        } else {
            // returning state back to normal if click is not handled
            tagsButton.controlState = .normal
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
        if let textField = obj.object as? AutoCompleteInput, textField == descriptionTextField {
            viewModel.descriptionDidEndEditing()
        } else if let textField = obj.object as? NSTextField, textField == durationTextField {
            viewModel.setDuration(textField.stringValue)
        }
    }

    func control(_ control: NSControl, textView: NSTextView, doCommandBy commandSelector: Selector) -> Bool {
        if textView == descriptionTextField.currentEditor() {
            return descriptionControl(doCommandBy: commandSelector)
        } else if textView == durationTextField.currentEditor() {
            return durationControl(doCommandBy: commandSelector)
        } else {
            return false
        }
    }

    private func descriptionControl(doCommandBy commandSelector: Selector) -> Bool {
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
                if success {
                    return true
                }
            } else {
                descriptionTextField.resetTable()
                return false
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

    private func durationControl(doCommandBy commandSelector: Selector) -> Bool {
        if commandSelector == #selector(insertNewline(_:)) {
            view.window?.makeFirstResponder(nil)
        }

        return false
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

    private func setupProjectButtonContextMenu() {
        if projectButton.isSelected {
            let menu = NSMenu()
            let remove = NSMenuItem(title: "Remove project", action: #selector(clearProject), keyEquivalent: "")
            menu.addItem(remove)
            projectButton.menu = menu
        } else {
            projectButton.menu = nil
        }
    }

    @objc
    private func clearProject() {
        viewModel.clearProject()
    }

    private func setupKeyViewLoop() {
        descriptionTextField.nextKeyView = durationTextField
        durationTextField.nextKeyView = projectButton
        projectButton.nextKeyView = tagsButton
        tagsButton.nextKeyView = billableButton
        billableButton.nextKeyView = descriptionTextField
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

    private func presentAutoComplete(window: AutoCompleteViewWindow, withContentView contentView: AutoCompleteView, fromPoint: NSPoint) {
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

        _ = contentView.defaultTextField.becomeFirstResponder()
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
                self.projectAutocompleteResignTime = Date().timeIntervalSince1970
                self.closeProjectAutoComplete()
            }
        }

        tagsAutocompleteDidResignObserver = NotificationCenter.default.addObserver(
            forName: NSWindow.didResignKeyNotification,
            object: tagsAutoCompleteWindow,
            queue: .main
        ) { [unowned self] _ in
            if self.tagsAutoCompleteWindow.isVisible {
                self.tagsAutocompleteResignTime = Date().timeIntervalSince1970
                self.closeTagsAutoComplete()
            }
        }
    }

    private func closeProjectAutoComplete() {
        projectAutoCompleteWindow.cancel()
        projectButton.controlState = .normal
        projectAutoCompleteView.clean()
    }

    private func closeTagsAutoComplete() {
        tagsAutoCompleteWindow.cancel()
        tagsButton.controlState = .normal
        tagsAutoCompleteView.clean()
    }
}

// MARK: - NSTextFieldDelegate

extension TimerViewController: NSTextFieldDelegate {
    //
}

// MARK: - AutoCompleteViewDelegate

extension TimerViewController: AutoCompleteViewDelegate {

    func shouldClose() {
        if projectAutoCompleteWindow.isVisible {
            closeProjectAutoComplete()
        } else if tagsAutoCompleteWindow.isVisible {
            closeTagsAutoComplete()
        }
    }

    func didTapOnCreateButton() {
        if projectAutoCompleteWindow.isVisible {
            showProjectCreationView()
        } else if tagsAutoCompleteWindow.isVisible {
            tagsAutocompleteDidTapOnCreateButton()
        }
    }

    private func showProjectCreationView() {
        projectCreationView.workspaceID = viewModel.workspaceID
        projectCreationView.timeEntryGUID = viewModel.timeEntryGUID
        projectCreationView.timeEntryIsBillable = viewModel.billableState == .on

        updateProjectAutocompleteWindowContent(with: projectCreationView, height: projectCreationView.suitableHeight)
        projectCreationView.setTitleAndFocus(projectAutoCompleteView.defaultTextField.stringValue)
    }

    private func updateProjectAutocompleteWindowContent(with view: NSView, height: CGFloat) {
        projectAutoCompleteWindow.contentView = view

        let fromPoint = NSPoint(x: projectButton.frame.minX, y: projectButton.frame.maxY)
        let windowRect = autoCompleteWindowRect(fromPoint: fromPoint)
        projectAutoCompleteWindow.setFrame(windowRect, display: false)
        projectAutoCompleteWindow.setFrameTopLeftPoint(windowRect.origin)

        projectAutoCompleteWindow.makeKey()
    }

    private func tagsAutocompleteDidTapOnCreateButton() {
        viewModel.createNewTag(withName: tagsAutoCompleteView.defaultTextField.stringValue)
        _ = tagsAutoCompleteView.defaultTextField.becomeFirstResponder()
    }
}

// MARK: - ProjectCreationViewDelegate

extension TimerViewController: ProjectCreationViewDelegate {
    func projectCreationDidCancel() {
        closeProjectAutoComplete()
    }

    func projectCreationDidAdd(with name: String, color: String, projectGUID: String) {
        closeProjectAutoComplete()

        // TODO: set project on UI
        // problem is we don't have library method to get project by GUID and pass it to view model
    }

    func projectCreationDidUpdateSize() {
        updateProjectAutocompleteWindowContent(with: projectCreationView, height: projectCreationView.suitableHeight)
    }
}
