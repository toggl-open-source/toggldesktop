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

    private lazy var projectAutoCompleteWindow: DropdownWindow = {
        return DropdownWindow(view: projectAutoCompleteView)
    }()
    private var projectAutoCompleteView = AutoCompleteView.xibView()
    private var projectAutocompleteDidResignObserver: Any?
    private var projectAutocompleteResignTime: TimeInterval = 0

    private lazy var projectCreationView: ProjectCreationView = {
        let view = ProjectCreationView.xibView()
        view.delegate = self
        return view
    }()

    private lazy var tagsAutoCompleteWindow: DropdownWindow = {
        return DropdownWindow(view: tagsAutoCompleteView)
    }()
    private var tagsAutoCompleteView = AutoCompleteView.xibView()
    private var tagsAutocompleteDidResignObserver: Any?
    private var tagsAutocompleteResignTime: TimeInterval = 0

    private var descriptionFieldHandler: TimerDescriptionFieldHandler!

    private var isEditingDescription: Bool {
        descriptionTextField.currentEditor() != nil
    }

    private var durationControl: TimerDurationControl!

    private enum Constants {
        static let emptyProjectButtonTooltip = NSLocalizedString("Select project (@)", comment: "Tooltip for timer project button")
        static let emptyTagsButtonTooltip = NSLocalizedString("Select tags (#)", comment: "Tooltip for timer tags button")
        static let billableOnTooltip = NSLocalizedString("Billable", comment: "Tooltip for timer billable button when On")
        static let billableOffTooltip = NSLocalizedString("Non-billable", comment: "Tooltip for timer billable button when Off")
        static let billableUnavailableTooltip = NSLocalizedString("Billable rates is not on your plan",
                                                                  comment: "Tooltip for timer billable button when disabled")

        static let durationControlSize = NSSize(width: 90, height: 30)
    }

    // MARK: - Outlets

    @IBOutlet weak var descriptionTextField: ResponderAutoCompleteInput! {
        didSet {
            descriptionFieldHandler = TimerDescriptionFieldHandler(textField: descriptionTextField)
        }
    }

    @IBOutlet weak var startButton: HoverButton! {
        didSet {
            startButton.activateOnEnterKey = true
        }
    }
    @IBOutlet weak var descriptionContainerBox: TimerContainerBox!
    @IBOutlet weak var trailingStackView: NSStackView!
    @IBOutlet weak var projectButton: SelectableButton!
    @IBOutlet weak var tagsButton: SelectableButton!
    @IBOutlet weak var billableButton: SelectableButton!
    @IBOutlet weak var buttonsStackView: NSStackView!

    // MARK: - Lifecycle

    override func viewDidLoad() {
        super.viewDidLoad()

        setupDescriptionField()
        setupDurationControl()

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
            return self?.durationControl.isEditing == true
        }

        // !!!: we're passing views into view model - refactor this someday
        // this is needed because current Autocomplete functionality
        // is tightly coupled with text input views
        viewModel.setDescriptionAutoCompleteInput(descriptionTextField)

        viewModel.projectDataSource.setup(with: projectAutoCompleteView)
        viewModel.tagsDataSource.setup(with: tagsAutoCompleteView)
    }

    override func viewDidAppear() {
        super.viewDidAppear()

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
            if self.durationControl.durationStringValue != duration {
                self.durationControl.durationStringValue = duration
            }
        }

        viewModel.onStartTimeChanged = { [unowned self] startTimeString, startDate in
            if self.durationControl.startTimeStringValue != startTimeString {
                self.durationControl.startTimeStringValue = startTimeString
            }
            self.durationControl.startDateValue = startDate
        }

        viewModel.onTagSelected = { [unowned self] isSelected in
            if case .tagsFilter = self.descriptionFieldHandler.state {
                self.descriptionFieldHandler.didSelectTag()
                self.analyticsTrackShortcutSelected(.tag)
            }

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
            switch self.descriptionFieldHandler.state {
            case .projectFilter:
                self.descriptionFieldHandler.didSelectProject()
                self.analyticsTrackShortcutSelected(.project)
            default:
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

    @objc
    func shortcutsOnboardingView() -> NSView {
        return buttonsStackView
    }

    @objc
    func shortcutsOnboardingPositioningRect() -> NSRect {
        // onboarding for shortcuts is presented between projectButton and tagsButton
        // that's why resulting rect should contain only those two buttons
        // !!!: we assume that buttons order won't change: project | tags | billable
        var rect = buttonsStackView.bounds
        rect.size.width = tagsButton.frame.maxX
        return rect
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
            presentProjectAutoComplete(from: .button(projectButton))
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
            tagsAutoCompleteView.isSearchFieldHidden = false
            viewModel.tagsDataSource.mode = .multiSelection
            tagsAutoCompleteView.filter(with: "")
            presentTagsAutoComplete(from: .button(tagsButton))
        } else {
            // returning state back to normal if click is not handled
            tagsButton.controlState = .normal
        }
    }

    @IBAction func billableButtonClicked(_ sender: Any) {
        viewModel.setBillable(!billableButton.isSelected)
    }

    override func mouseDown(with event: NSEvent) {
        view.window?.makeFirstResponder(nil)
    }

    // MARK: - Description field handling

    private func setupDescriptionField() {
        descriptionTextField.displayMode = .fullscreen
        descriptionContainerBox.setup(for: descriptionTextField)
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
                presentProjectAutoComplete(from: .textField(descriptionTextField))
            }
            projectAutoCompleteView.filter(with: filterText)

        case .tagsFilter(let filterText):
            if tagsAutoCompleteWindow.isVisible == false {
                tagsAutoCompleteView.isSearchFieldHidden = true
                viewModel.tagsDataSource.mode = .singleSelection
                presentTagsAutoComplete(from: .textField(descriptionTextField))
            }
            tagsAutoCompleteView.filter(with: filterText)

        case .autocompleteFilter(let filterText):
            viewModel.filterAutocomplete(with: filterText)
        }
    }

    private func resetFromDescriptionFieldState(_ state: TimerDescriptionFieldHandler.State) {
        switch state {
        case .autocompleteFilter:
            descriptionTextField.toggleList(false)
        case .projectFilter:
            closeProjectAutoComplete()
        case .tagsFilter:
            closeTagsAutoComplete()
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

        case .tagsAutoCompleteTableHandleEvent(let event):
            _ = tagsAutoCompleteView.tableView?.handleKeyboardEvent(event)

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
                $0.selectedBackgroundColor = Color.green.color
        }
        billableButton.isActiveOnClick = false

        startButton.hoverImage = NSImage(named: "start-timer-button-hover")

        projectButton.toolTip = Constants.emptyProjectButtonTooltip
        tagsButton.toolTip = Constants.emptyTagsButtonTooltip
        billableButton.toolTip = Constants.billableOffTooltip
    }

    private func setupDurationControl() {
        durationControl = TimerDurationControl.xibView()
        durationControl.widthAnchor.constraint(equalToConstant: Constants.durationControlSize.width).isActive = true
        durationControl.heightAnchor.constraint(equalToConstant: Constants.durationControlSize.height).isActive = true
        trailingStackView.insertView(durationControl, at: 0, in: .leading)

        durationControl.onDurationTextChange = { [unowned self] text in
            self.viewModel.setDuration(text)
        }

        durationControl.onStartTextChange = { [unowned self] text in
            self.viewModel.setStartTime(text)
        }

        durationControl.onStartDateChange = { [unowned self] startDate in
            self.viewModel.setStartDate(startDate)
        }

        durationControl.onPerformAction = { [unowned self] action in
            switch action {
            case .enterPress:
                view.window?.makeFirstResponder(nil)
                if !self.viewModel.isRunning {
                    self.viewModel.startStopAction()
                }
                return true
            }
        }
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
        descriptionTextField.nextKeyView = durationControl
        durationControl.nextKeyView = startButton
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

    private func presentProjectAutoComplete(from: AutocompleteSourceViewType) {
        presentAutoComplete(window: projectAutoCompleteWindow,
                            withContentView: projectAutoCompleteView,
                            from: from)
        viewModel.projectDataSource.sizeToFit()
    }

    private func presentTagsAutoComplete(from: AutocompleteSourceViewType) {
        presentAutoComplete(window: tagsAutoCompleteWindow,
                            withContentView: tagsAutoCompleteView,
                            from: from)
        viewModel.tagsDataSource.sizeToFit()
    }

    private func presentAutoComplete(window: DropdownWindow,
                                     withContentView contentView: AutoCompleteView,
                                     from: AutocompleteSourceViewType) {
        window.contentView = contentView
        window.setContentSize(contentView.frame.size)

        if !window.isVisible {
            let windowRect = autoCompleteWindowRect(fromView: from.sourceView, offset: from.offset)
            window.setFrame(windowRect, display: false)
            window.setFrameTopLeftPoint(windowRect.origin)

            if view.window != window {
                view.window?.addChildWindow(window, ordered: .above)
            }

            if from.makeWindowKey {
                durationControl.closeDropdown()

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
            let hasChildWindows = self.projectAutoCompleteWindow.childWindows?.isEmpty == false
            if self.projectAutoCompleteWindow.isVisible && !hasChildWindows {
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
        descriptionFieldHandler.didCloseTagsDropdown()
    }

    // MARK: - Other

    @objc
    private func clearProject() {
        viewModel.clearProject()
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
        updateProjectAutocompleteWindowContent(with: projectCreationView)

        switch descriptionFieldHandler.state {
        case .projectFilter(let filter):
            projectCreationView.setTitleAndFocus(filter)
        default:
            projectCreationView.setTitleAndFocus(projectAutoCompleteView.defaultTextField.stringValue)
        }
    }

    private func updateProjectAutocompleteWindowContent(with view: NSView) {
        let prevFrame = projectAutoCompleteWindow.frame

        projectAutoCompleteWindow.contentView = view

        let windowRect = autoCompleteWindowRect(fromView: projectButton)
        projectAutoCompleteWindow.setFrame(windowRect, display: false)
        projectAutoCompleteWindow.setFrameTopLeftPoint(NSPoint(x: prevFrame.minX, y: prevFrame.maxY))

        projectAutoCompleteWindow.makeKey()
    }

    private func tagsAutocompleteDidTapOnCreateButton() {
        let tagName: String
        switch descriptionFieldHandler.state {
        case .tagsFilter(let filter):
            tagName = filter
            descriptionFieldHandler.didSelectTag()
            analyticsTrackShortcutCreated(.tag)

        default:
            tagName = tagsAutoCompleteView.defaultTextField.stringValue
        }

        viewModel.createNewTag(withName: tagName)
        _ = tagsAutoCompleteView.defaultTextField.becomeFirstResponder()
    }
}

// MARK: - ProjectCreationViewDelegate

extension TimerViewController: ProjectCreationViewDelegate {
    func projectCreationDidCancel() {
        closeProjectAutoComplete()
    }

    func projectCreationDidAdd(newProject: Project) {
        switch descriptionFieldHandler.state {
        case .projectFilter:
            descriptionFieldHandler.didSelectProject()
            analyticsTrackShortcutCreated(.project)
        default:
            closeProjectAutoComplete()
        }

        viewModel.didCreateNewProject(newProject)
    }

    func projectCreationDidUpdateSize() {
        updateProjectAutocompleteWindowContent(with: projectCreationView)
    }
}

// MARK: - Analytics

extension TimerViewController {

    private typealias Shortcut = TimerDescriptionFieldHandler.Shortcut

    private func analyticsTrackShortcutSelected(_ shortcut: Shortcut) {
        switch shortcut {
        case .project:
            DesktopLibraryBridge.shared().trackTimerShortcut(TimerShortcutActionTypeProjectSelected)
        case .tag:
            DesktopLibraryBridge.shared().trackTimerShortcut(TimerShortcutActionTypeTagSelected)
        }
    }

    private func analyticsTrackShortcutCreated(_ shortcut: Shortcut) {
        switch shortcut {
        case .project:
            DesktopLibraryBridge.shared().trackTimerShortcut(TimerShortcutActionTypeProjectCreated)
        case .tag:
            DesktopLibraryBridge.shared().trackTimerShortcut(TimerShortcutActionTypeTagCreated)
        }
    }
}
