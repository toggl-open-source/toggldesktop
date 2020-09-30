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

    private var descriptionFieldHandler: TimerDescriptionFieldHandler!

    private var isEditingDescription: Bool {
        descriptionTextField.currentEditor() != nil
    }

    private enum Constants {
        static let emptyProjectButtonTooltip = NSLocalizedString("Select project", comment: "Tooltip for timer project button")
        static let emptyTagsButtonTooltip = NSLocalizedString("Select tags", comment: "Tooltip for timer tags button")
        static let billableOnTooltip = NSLocalizedString("Billable", comment: "Tooltip for timer billable button when On")
        static let billableOffTooltip = NSLocalizedString("Non-billable", comment: "Tooltip for timer billable button when Off")
        static let billableUnavailableTooltip = NSLocalizedString("Billable rates is not on your plan",
                                                                  comment: "Tooltip for timer billable button when disabled")

        static let projectShortcutSymbol = "@"
        static let tagShortcutSymbol = "#"
        static let autocompleteLengthTriggerCount = 2

        static let descriptionDropdownOffset = NSPoint(x: 8, y: 6)
        static let buttonsDropdownOffset = NSPoint(x: 0, y: 2)
    }

    // MARK: - Outlets

    @IBOutlet weak var descriptionTextField: ResponderAutoCompleteInput! {
        didSet {
            descriptionFieldHandler = TimerDescriptionFieldHandler(textField: descriptionTextField)
        }
    }

    @IBOutlet weak var startButton: NSHoverButton!
    @IBOutlet weak var descriptionContainerBox: TimerContainerBox!
    @IBOutlet weak var durationContainerBox: TimerContainerBox!
    @IBOutlet weak var durationTextField: ResponderTextField!
    @IBOutlet weak var projectButton: SelectableButton!
    @IBOutlet weak var tagsButton: SelectableButton!
    @IBOutlet weak var billableButton: SelectableButton!

    // MARK: - Lifecycle

    override func viewDidLoad() {
        super.viewDidLoad()

        setupDescriptionField()

        durationTextField.responderDelegate = durationContainerBox

        projectAutoCompleteView.delegate = self
        tagsAutoCompleteView.delegate = self

        configureAppearance()
        setupProjectButtonContextMenu()
        setupKeyViewLoop()

        setupBindings()

        viewModel.isEditingDescription = { [weak self] in
            return self?.isEditingDescription == true
        }

        viewModel.isEditingDuration = { [weak self] in
            return self?.durationTextField.currentEditor() != nil
        }
    }

    override func viewDidAppear() {
        super.viewDidAppear()

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
            self.updateStartButton(forRunningState: isRunning)
            NotificationCenter.default.post(name: NSNotification.Name(rawValue: kStartButtonStateChange),
                                            object: NSNumber(value: isRunning))
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
            self.tagsButton.toolTip = isSelected ? nil : Constants.emptyTagsButtonTooltip
        }

        viewModel.onProjectUpdated = { [unowned self] project in
            if let project = project {
                self.projectButton.selectedBackgroundColor = project.color.withAlphaComponent(0.3)
                self.projectButton.attributedTitle = project.attributedTitle
                self.projectButton.image = nil
                self.projectButton.isSelected = true
                self.projectButton.toolTip = nil
            } else {
                self.projectButton.title = ""
                self.projectButton.image = NSImage(named: "project-button")
                self.projectButton.isSelected = false
                self.projectButton.toolTip = Constants.emptyProjectButtonTooltip
            }
            self.setupProjectButtonContextMenu()
        }

        viewModel.onProjectSelected = { [unowned self] project in
            if self.isEditingDescription {
                self.descriptionFieldHandler.didSelectProject()
            } else {
                self.closeProjectAutoComplete()
            }
        }

        viewModel.onBillableChanged = { [unowned self] billable in
            self.billableButton.isEnabled = billable != .unavailable
            self.billableButton.isSelected = billable == .on

            switch billable {
            case .on:
                self.billableButton.toolTip = Constants.billableOnTooltip
            case .off:
                self.billableButton.toolTip = Constants.billableOffTooltip
            case .unavailable:
                self.billableButton.toolTip = Constants.billableUnavailableTooltip
            }

            // billable button is excluded from key view loop when disabled
            // so need to update it when it is changed
            self.setupKeyViewLoop()
        }

        viewModel.onTouchBarUpdateRunningItem = { entry in
            if #available(macOS 10.12.2, *) {
                TouchBarService.shared.updateRunningItem(entry)
            }
        }
    }

    @objc
    func focusDescriptionField() {
        if viewModel.isRunning {
            return
        }

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
            projectAutoCompleteView.isSearchFieldHidden = false
            projectAutoCompleteView.filter(with: "")
            presentProjectAutoComplete(from: projectButton, offset: Constants.buttonsDropdownOffset)
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

    override func mouseDown(with event: NSEvent) {
        super.mouseDown(with: event)
        view.window?.makeFirstResponder(nil)
        closeProjectAutoComplete()
        closeTagsAutoComplete()
    }

    // MARK: - Description field handling

    private func setupDescriptionField() {
        descriptionTextField.displayMode = .fullscreen
        descriptionTextField.responderDelegate = descriptionContainerBox
        descriptionTextField.delegate = descriptionFieldHandler

        descriptionFieldHandler.onStateChanged = { [weak self] newState, oldState in
            self?.handleDescriptionFieldStateChange(from: oldState, to: newState)
        }

        descriptionFieldHandler.onPerformAction = { [weak self] action in
            return self?.handleDescriptionFieldAction(action) ?? false
        }
    }

    private func handleDescriptionFieldStateChange(from oldState: TimerDescriptionFieldHandler.State,
                                                   to newState: TimerDescriptionFieldHandler.State) {

        if newState.equalCase(to: oldState) == false {
            resetFromDescriptionFieldState(oldState)
        }

        // view model needs to save description in any case
        viewModel.setDescription(descriptionTextField.stringValue)

        switch newState {
        case .descriptionUpdate:
            break

        case .projectFilter(let filterText):
            if projectAutoCompleteWindow.isVisible == false {
                projectAutoCompleteView.isSearchFieldHidden = true
                presentProjectAutoComplete(from: descriptionTextField,
                                           makeKey: false,
                                           offset: Constants.descriptionDropdownOffset)
            }
            projectAutoCompleteView.filter(with: String(filterText))

        case .autocompleteFilter(let filterText):
            viewModel.filterAutocomplete(with: filterText)
        }
    }

    private func resetFromDescriptionFieldState(_ state: TimerDescriptionFieldHandler.State) {
        switch state {
        case .autocompleteFilter:
            descriptionTextField.resetTable()
        case .projectFilter:
            closeProjectAutoComplete()
        case .descriptionUpdate:
            break
        }
    }

    private func handleDescriptionFieldAction(_ action: TimerDescriptionFieldHandler.Action) -> Bool {
        switch action {
        case .startTimeEntry:
            if !viewModel.isRunning {
                viewModel.startStopAction()
            }

        case .endEditing:
            viewModel.descriptionDidEndEditing()

        case .unfocus:
            view.window?.makeFirstResponder(nil)

        case .projectAutoCompleteTableHandleEvent(let event):
            _ = projectAutoCompleteView.tableView?.handleKeyboardEvent(event)

        case .autoCompleteTableSelectNext:
            descriptionTextField.autocompleteTableView.nextItem()

        case .autoCompleteTableSelectPrevious:
            descriptionTextField.autocompleteTableView.previousItem()

        case .autoCompleteSelectCurrent:
            let lastSelectedIndex = descriptionTextField.autocompleteTableView.lastSelected
            if lastSelectedIndex >= 0 {
                _ = viewModel.selectDescriptionAutoCompleteItem(at: lastSelectedIndex)
            }
        }

        return true
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

        startButton.hoverImage = NSImage(named: "start-timer-button-hover")

        projectButton.toolTip = Constants.emptyProjectButtonTooltip
        tagsButton.toolTip = Constants.emptyTagsButtonTooltip
        billableButton.toolTip = Constants.billableOffTooltip
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

    private func setupKeyViewLoop() {
        descriptionTextField.nextKeyView = durationTextField
        durationTextField.nextKeyView = startButton
        startButton.nextKeyView = projectButton
        projectButton.nextKeyView = tagsButton

        if billableButton.isEnabled {
            tagsButton.nextKeyView = billableButton
            billableButton.nextKeyView = descriptionTextField
        } else {
            tagsButton.nextKeyView = descriptionTextField
        }
    }

    private func updateStartButton(forRunningState isRunning: Bool) {
        if isRunning {
            startButton.toolTip = "Stop"
            startButton.image = NSImage(named: "stop-timer-button")
            startButton.alternateImage = NSImage(named: "stop-timer-button-onpress")
            startButton.hoverImage = NSImage(named: "stop-timer-button-hover")
        } else {
            startButton.toolTip = "Start"
            startButton.image = NSImage(named: "start-timer-button")
            startButton.alternateImage = NSImage(named: "start-timer-button-onpress")
            startButton.hoverImage = NSImage(named: "start-timer-button-hover")
        }
        startButton.state = .off
    }

    // MARK: - Autocomplete/Dropdown

    private func presentProjectAutoComplete(from: NSView, makeKey: Bool = true, offset: NSPoint = .zero) {
        presentAutoComplete(window: projectAutoCompleteWindow,
                            withContentView: projectAutoCompleteView,
                            fromView: from,
                            makeKey: makeKey,
                            offset: offset)
        viewModel.projectDataSource.sizeToFit()
    }

    private func presentTagsAutoComplete() {
        presentAutoComplete(window: tagsAutoCompleteWindow,
                            withContentView: tagsAutoCompleteView,
                            fromView: tagsButton,
                            offset: Constants.buttonsDropdownOffset)
        viewModel.tagsDataSource.sizeToFit()
    }

    private func presentAutoComplete(window: AutoCompleteViewWindow,
                                     withContentView contentView: AutoCompleteView,
                                     fromView: NSView,
                                     makeKey: Bool = true,
                                     offset: NSPoint = .zero) {
        window.contentView = contentView
        window.setContentSize(contentView.frame.size)

        if !window.isVisible {
            let windowRect = autoCompleteWindowRect(fromView: fromView, offset: offset)
            window.setFrame(windowRect, display: false)
            window.setFrameTopLeftPoint(windowRect.origin)

            if view.window != window {
                view.window?.addChildWindow(window, ordered: .above)
            }

            if makeKey {
                window.makeKeyAndOrderFront(nil)
                window.makeFirstResponder(contentView)
            }
        }

        _ = contentView.defaultTextField.becomeFirstResponder()
    }

    private func autoCompleteWindowRect(fromView: NSView, offset: NSPoint = .zero) -> NSRect {
        let padding: CGFloat = 6
        // passing random height value as it should be updated with to its content size later
        let initialHeight: CGFloat = 100
        let windowSize = NSSize(width: view.frame.width - padding * 2, height: initialHeight)
        var rect = NSRect(origin: .zero, size: windowSize)

        let fromPoint = NSPoint(x: fromView.bounds.minX + offset.x,
                                y: fromView.bounds.maxY + offset.y)
        let windowPoint = fromView.convert(fromPoint, to: nil)
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
        descriptionFieldHandler.didCloseProjectDropdown()
    }

    private func closeTagsAutoComplete() {
        tagsAutoCompleteWindow.cancel()
        tagsButton.controlState = .normal
        tagsAutoCompleteView.clean()
    }

    // MARK: - Other

    @objc
    private func clearProject() {
        viewModel.clearProject()
    }
}

// MARK: - NSTextFieldDelegate

extension TimerViewController: NSTextFieldDelegate {

    func controlTextDidEndEditing(_ obj: Notification) {
        if let textField = obj.object as? NSTextField, textField == durationTextField {
            viewModel.setDuration(textField.stringValue)
        }
    }

    func control(_ control: NSControl, textView: NSTextView, doCommandBy commandSelector: Selector) -> Bool {
        if textView == durationTextField.currentEditor() {
            return durationControl(doCommandBy: commandSelector)
        } else {
            return false
        }
    }

    private func durationControl(doCommandBy commandSelector: Selector) -> Bool {
        if commandSelector == #selector(insertNewline(_:)) {
            view.window?.makeFirstResponder(nil)
            if !viewModel.isRunning {
                viewModel.startStopAction()
            }
        }

        return false
    }
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
        projectCreationView.setTimeEntry(guid: viewModel.timeEntryGUID,
                                         workspaceID: viewModel.workspaceID,
                                         isBillable: viewModel.billableState == .on)
        updateProjectAutocompleteWindowContent(with: projectCreationView, height: projectCreationView.suitableHeight)
        projectCreationView.setTitleAndFocus(projectAutoCompleteView.defaultTextField.stringValue)
    }

    private func updateProjectAutocompleteWindowContent(with view: NSView, height: CGFloat) {
        projectAutoCompleteWindow.contentView = view

        let windowRect = autoCompleteWindowRect(fromView: projectButton)
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
