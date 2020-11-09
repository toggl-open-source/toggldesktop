//
//  TimerViewModel.swift
//  TogglDesktop
//
//  Created by Andrew Nester on 28.07.2020.
//  Copyright © 2020 Toggl. All rights reserved.
//

import Foundation

final class TimerViewModel: NSObject {

    private(set) var entryDescription: String = "" {
        didSet {
            guard entryDescription != oldValue else { return }
            timeEntry.entryDescription = entryDescription
            onDescriptionChanged?(entryDescription)
        }
    }

    private(set) var durationString: String = "" {
        didSet {
            onDurationChanged?(durationString)
        }
    }

    private(set) var startTimeString: String = "" {
        didSet {
            onStartTimeChanged?(startTimeString, timeEntry.started ?? Date())
        }
    }

    private(set) var isRunning: Bool = false {
        didSet {
            onIsRunning?(isRunning)
        }
    }

    enum BillableState {
        case on
        case off
        case unavailable
    }

    private(set) var billableState: BillableState = .unavailable {
        didSet {
            guard billableState != oldValue else { return }
            let isBillable = billableState == .on

            if timeEntry.billable != isBillable {
                timeEntry.billable = isBillable

                if timeEntry.isRunning(), let timeEntryGUID = timeEntry.guid {
                    save(isBillable: isBillable, forEntryWithGUID: timeEntryGUID)
                }
            }

            onBillableChanged?(billableState)
        }
    }

    private var selectedTags: [String] = [] {
        didSet {
            guard selectedTags != oldValue else { return }
            if tagsDataSource.autoCompleteView != nil {
                tagsDataSource.updateSelectedTags(selectedTags.map { Tag(name: $0) })
            }

            let currentTags: [String] = timeEntry.tags ?? []
            if currentTags != selectedTags {
                timeEntry.tags = selectedTags

                if timeEntry.isRunning(), let timeEntryGUID = timeEntry.guid {
                    save(tags: selectedTags, forEntryWithGUID: timeEntryGUID)
                }
            }

            onTagSelected?(!selectedTags.isEmpty)
        }
    }

    var timeEntryGUID: String? { timeEntry.guid }
    var workspaceID: UInt64 { timeEntry.workspaceID }

    // Bindings
    var onIsRunning: ((Bool) -> Void)?
    var onDescriptionChanged: ((String) -> Void)?
    var onDurationChanged: ((String) -> Void)?
    var onStartTimeChanged: ((String, Date) -> Void)?
    var onTagSelected: ((Bool) -> Void)?
    var onProjectUpdated: ((Project?) -> Void)?
    /// Called when project was selected via autocomplete data source
    var onProjectSelected: ((Project?) -> Void)?
    var onBillableChanged: ((BillableState) -> Void)?
    var onDescriptionFocusChanged: ((Bool) -> Void)?
    var onTouchBarUpdateRunningItem: ((TimeEntryViewItem) -> Void)?

    // View outputs
    var isEditingDescription: (() -> Bool)?
    var isEditingDuration: (() -> Bool)?

    // !!!: it is needed for EditorViewController. Maybe it can be removed later.
    /// Timer tick notification will be posted every second if there is a running time entry.
    static let timerOnTickNotification = NSNotification.Name("TimerForRunningTimeEntryOnTicket")

    private var timeEntry = TimeEntryViewItem()
    private var notificationObservers: [AnyObject] = []

    var descriptionDataSource = LiteAutoCompleteDataSource(notificationName: kDisplayMinitimerAutocomplete)

    var projectDataSource = ProjectDataSource(items: ProjectStorage.shared.items,
                                              updateNotificationName: .ProjectStorageChangedNotification)

    var tagsDataSource = TagDataSource(items: TagStorage.shared.tags,
                                       updateNotificationName: .TagStorageChangedNotification)

    private var timer: Timer!

    private var actionsUsedBeforeStart: Set<TimerEditActionType> = Set()

    // MARK: - Lifecycle

    override init() {
        super.init()
        setupNotificationObservers()

        timer = Timer.scheduledTimer(timeInterval: 1.0,
                                     target: self,
                                     selector: #selector(timerFired(_:)),
                                     userInfo: nil,
                                     repeats: true)

        projectDataSource.delegate = self

        tagsDataSource.delegate = self
        tagsDataSource.tagDelegate = self
    }

    deinit {
        cancelNotificationObservers()
        timer.invalidate()
    }

    // MARK: - Public

    func startStopAction() {
        if timeEntry.isRunning() {
            stopTimeEntry()
            isRunning = false
        } else {
            startTimeEntry()
            isRunning = true
        }
    }

    func setDescriptionAutoCompleteInput(_ input: AutoCompleteInput) {
        descriptionDataSource.input = input
        descriptionDataSource.setFilter("")
        input.autocompleteTableView.delegate = self
    }

    func selectDescriptionAutoCompleteItem(at index: Int) -> Bool {
        guard let item = descriptionDataSource.item(at: index) else {
            return false
        }
        fillEntry(fromDescriptionAutocomplete: item)

        descriptionDataSource.input?.resetTable()
        descriptionDataSource.clearFilter()

        return true
    }

    func prepareData() {
        fetchTags()
        updateBillableStatus()
    }

    func setDescription(_ description: String) {
        entryDescription = description
    }

    func filterAutocomplete(with filter: String) {
        descriptionDataSource.setFilter(filter)
        descriptionDataSource.input?.autocompleteTableView.resetSelected()
    }

    func descriptionDidEndEditing() {
        if timeEntry.isRunning() {
            saveCurrentDescription()
        }
    }

    func setBillable(_ isOn: Bool) {
        billableState = isOn ? .on : .off

        actionsUsedBeforeStart.insert(TimerEditActionTypeBillable)
    }

    func setDuration(_ duration: String) {
        guard durationString != duration else { return }

        if !duration.isEmpty {
            timeEntry.started = startDate(fromDurationString: duration)
            reloadTimeStrings()
        } else if !timeEntry.isRunning() {
            timeEntry.started = nil
            durationString = ""
            startTimeString = ""
        }

        if timeEntry.isRunning(), let guid = timeEntry.guid {
            save(duration: duration, forEntryWithGUID: guid)
        }

        actionsUsedBeforeStart.insert(TimerEditActionTypeDuration)
    }

    func setStartTime(_ startString: String) {
        guard !startString.isEmpty else { return }

        var newTimestamp = DesktopLibraryBridge.shared().timestamp(from: startString)
        guard newTimestamp > 0 else { return }

        if timeEntry.isRunning(), let startDate = timeEntry.started {
            // because start time string can have only hours and minutes,
            // we are manually adding remainder seconds from current time entry
            newTimestamp += TimeInterval(startDate.seconds)
        } else {
            // when not running we round the duration to minutes for ease of use
            newTimestamp += TimeInterval(Date().seconds)
        }

        var newDate = Date(timeIntervalSince1970: newTimestamp)
        // `newDate` is always "today" or "yesterday"
        if let currentTime = timeEntry.started, newDate.daysBetween(endDate: currentTime) != 0 {
            // update to be the same day as old start time
            newDate = Date.combine(dayFrom: timeEntry.started, withTimeFrom: newDate) ?? newDate
        }

        timeEntry.started = newDate
        reloadTimeStrings()

        if timeEntry.isRunning(), let guid = timeEntry.guid {
            DesktopLibraryBridge.shared().updateTimeEntryWithStart(atTimestamp: timeEntry.started.timeIntervalSince1970,
                                                                   guid: guid,
                                                                   keepEndTimeFixed: false)
        }
    }

    /// Sets date for current time entry. Only year, month and day components are used to update the current time entry start time.
    /// So use this method only to set the "start day".
    func setStartDate(_ startDate: Date) {
        guard startDate < Date() else { return }

        if timeEntry.started == nil {
            timeEntry.started = startDate
        }

        guard var combinedDateTime = Date.combine(dayFrom: startDate, withTimeFrom: timeEntry.started) else {
            return
        }

        if !timeEntry.isRunning() {
            // when not running we round the duration to minutes for ease of use
            combinedDateTime = combinedDateTime.addingTimeInterval(
                TimeInterval(Date().seconds - combinedDateTime.seconds)
            )
        }

        timeEntry.started = combinedDateTime < Date() ? combinedDateTime : Date()
        reloadTimeStrings()

        if timeEntry.isRunning(), let guid = timeEntry.guid {
            DesktopLibraryBridge.shared().updateTimeEntryWithStart(atTimestamp: timeEntry.started.timeIntervalSince1970,
                                                                   guid: guid,
                                                                   keepEndTimeFixed: false)
        }
    }

    func createNewTag(withName name: String) {
        TagStorage.shared.addNewTag(Tag(name: name))
        selectedTags.append(name)
        tagsDataSource.filter(with: name)
    }

    func clearProject() {
        timeEntry.projectID = 0
        timeEntry.projectGUID = ""
        timeEntry.taskID = 0
        timeEntry.projectAndTaskLabel = ""
        timeEntry.taskLabel = ""
        timeEntry.projectLabel = ""
        timeEntry.clientLabel = ""
        timeEntry.projectColor = ""

        if timeEntry.isRunning() {
            save(taskID: timeEntry.taskID, projectID: timeEntry.projectID, projectGUID: timeEntry.projectGUID, forEntryWithGUID: timeEntry.guid)
        }

        onProjectUpdated?(Project(timeEntry: timeEntry))

        actionsUsedBeforeStart.insert(TimerEditActionTypeProject)
    }

    func didCreateNewProject(_ project: Project) {
        if !isRunning {
            // when TE is running, the project is set on Library level
            clearProject()
        }

        timeEntry.projectGUID = project.guid
        timeEntry.projectColor = project.colorHex
        timeEntry.projectLabel = project.name
        timeEntry.taskLabel = project.taskName
        timeEntry.clientLabel = project.clientName

        onProjectUpdated?(project)
    }

    // MARK: - Private

    private func startTimeEntry() {
        if !durationString.isEmpty {
            timeEntry.started = startDate(fromDurationString: durationString)
        }

        NotificationCenter.default.post(name: NSNotification.Name(rawValue: kForceCloseEditPopover), object: nil)
        NotificationCenter.default.post(name: NSNotification.Name(rawValue: kCommandNew), object: timeEntry, userInfo: nil)

        descriptionDataSource.input?.resetTable()
        descriptionDataSource.clearFilter()

        analyticsTrackTimerStart()
    }

    private func stopTimeEntry() {
        NotificationCenter.default.post(name: NSNotification.Name(rawValue: kCommandStop), object: nil, userInfo: nil)
    }

    private func saveCurrentDescription(trackAnalytics: Bool = true) {
        if let timeEntryGUID = timeEntry.guid {
            DesktopLibraryBridge.shared().updateTimeEntry(withDescription: entryDescription, guid: timeEntryGUID)
            if trackAnalytics {
                DesktopLibraryBridge.shared().trackTimerEdit(using: TimerEditActionTypeDescription)
            }
        }
    }

    private func save(duration: String, forEntryWithGUID entryGUID: String) {
        DesktopLibraryBridge.shared().updateTimeEntry(withDuration: duration, guid: entryGUID)
        DesktopLibraryBridge.shared().trackTimerEdit(using: TimerEditActionTypeDuration)
    }

    private func save(taskID: UInt64, projectID: UInt64, projectGUID: String?, forEntryWithGUID entryGUID: String) {
        DesktopLibraryBridge.shared().setProjectForTimeEntryWithGUID(entryGUID,
                                                                     taskID: taskID,
                                                                     projectID: projectID,
                                                                     projectGUID: projectGUID)
        DesktopLibraryBridge.shared().trackTimerEdit(using: TimerEditActionTypeProject)
    }

    private func save(isBillable: Bool, forEntryWithGUID entryGUID: String) {
        DesktopLibraryBridge.shared().setBillableForTimeEntryWithTimeEntryGUID(entryGUID, isBillable: isBillable)
        DesktopLibraryBridge.shared().trackTimerEdit(using: TimerEditActionTypeBillable)
    }

    private func save(tags: [String], forEntryWithGUID entryGUD: String) {
        DesktopLibraryBridge.shared().updateTimeEntry(withTags: tags, guid: entryGUD)
        DesktopLibraryBridge.shared().trackTimerEdit(using: TimerEditActionTypeTags)
    }

    private func fetchTags() {
        var workspaceID = timeEntry.workspaceID
        if workspaceID <= 0 {
            workspaceID = DesktopLibraryBridge.shared().defaultWorkspaceID()
            timeEntry.workspaceID = workspaceID
        }
        DesktopLibraryBridge.shared().fetchTags(forWorkspaceID: workspaceID)
    }

    private func updateBillableStatus() {
        var workspaceID = timeEntry.workspaceID
        if workspaceID <= 0 {
            workspaceID = DesktopLibraryBridge.shared().defaultWorkspaceID()
            timeEntry.workspaceID = workspaceID
        }
        timeEntry.canSeeBillable = DesktopLibraryBridge.shared().canSeeBillable(forWorkspaceID: workspaceID)

        if timeEntry.canSeeBillable {
            billableState = timeEntry.billable ? .on : .off
        } else {
            billableState = .unavailable
        }
    }

    private func reloadTimeStrings() {
        startTimeString = timeString(fromDate: timeEntry.started)
        durationString = DesktopLibraryBridge.shared().convertDuraton(
            inSecond: Int64(Date().timeIntervalSince(timeEntry.started))
        )
    }

    private func startDate(fromDurationString durationString: String) -> Date {
        if durationString.isEmpty {
            return Date()
        } else {
            let durationSec = DesktopLibraryBridge.shared().seconds(fromDurationString: durationString)
            return Date(timeIntervalSinceNow: Double(-durationSec))
        }
    }

    private func timeString(fromDate date: Date?) -> String {
        guard let date = date else {
            return ""
        }
        return DesktopLibraryBridge.shared().formatTime(date.timeIntervalSince1970) ?? ""
    }

    @objc
    private func timerFired(_ timer: Timer) {
        guard timeEntry.isRunning() else { return }

        let isDurationEditing = isEditingDuration?() ?? true
        if !isDurationEditing {
            durationString = DesktopLibraryBridge.shared().convertDuraton(inSecond: timeEntry.duration_in_seconds)
        }

        NotificationCenter.default.post(name: Self.timerOnTickNotification, object: nil)
    }

    private func updateTimerState(with newTimeEntry: TimeEntryViewItem?) {
        let entry: TimeEntryViewItem
        if let timeEntry = newTimeEntry {
            entry = timeEntry
        } else if isRunning {
            // e.g. TE was removed
            entry = TimeEntryViewItem()
        } else {
            return
        }

        let isNewWorkspace = entry.workspaceID != timeEntry.workspaceID
        let isNewEntry = entry.guid != timeEntry.guid

        timeEntry = entry

        if isNewEntry {
            entryDescription = entry.entryDescription ?? ""
            durationString = entry.duration ?? ""
            startTimeString = entry.startTimeString ?? ""
            onDescriptionFocusChanged?(false)

        } else if entry.isRunning() {
            let isDescriptionEditing = isEditingDescription?() ?? true
            if !isDescriptionEditing {
                entryDescription = entry.entryDescription ?? ""
            }

            let isDurationEditing = isEditingDuration?() ?? true
            if durationString.isEmpty || !isDurationEditing {
                durationString = entry.duration ?? ""
            }

            if startTimeString != entry.startTimeString {
                startTimeString = entry.startTimeString ?? ""
            }

        } else {
            // don't update text fields if user haven't yet started TE
            // so we don't interfere with his editing
        }

        isRunning = entry.isRunning()
        selectedTags = entry.tags ?? []

        if entry.isRunning() {
            onTouchBarUpdateRunningItem?(entry)
        }

        onProjectUpdated?(Project(timeEntry: timeEntry))

        if isNewWorkspace {
            fetchTags()
        }

        updateBillableStatus()
    }

    private func focusTimer() {
        onDescriptionFocusChanged?(true)
    }

    private func timeEntryOnStop() {
        // updating description in case it wasn't yet updated
        // e.g. if TE was stopped by keyboard shortcut
        // and Timer haven't yet updated the contents of description field
        if !entryDescription.isEmpty {
            saveCurrentDescription(trackAnalytics: false)
        }

        timeEntry = TimeEntryViewItem()

        isRunning = false
        entryDescription = ""
        durationString = ""
        startTimeString = ""
        selectedTags = []
        fetchTags()
        updateBillableStatus()
        focusTimer()
        actionsUsedBeforeStart = Set()

        onProjectUpdated?(nil)
    }

    private func timeEntryContinue() {
        // saving description in case it wasn't yet updated
        if !entryDescription.isEmpty {
            saveCurrentDescription(trackAnalytics: false)
        }
    }

    private func fillEntry(fromDescriptionAutocomplete autocompleteItem: AutocompleteItem) {
        // User has selected an autocomplete item.
        // It could be a time entry, a task or a project.

        if let newDescription = autocompleteItem.description {
            entryDescription = newDescription
            if timeEntry.isRunning() {
                saveCurrentDescription()
            }
        }

        fillEntryProject(from: autocompleteItem)

        selectedTags = autocompleteItem.tags ?? []

        if timeEntry.canSeeBillable {
            billableState = autocompleteItem.billable ? .on : .off
        } else {
            billableState = .unavailable
        }
    }

    private func fillEntryProject(from autocompleteItem: AutocompleteItem) {
        let isNewWorkspace = autocompleteItem.workspaceID != timeEntry.workspaceID

        timeEntry.workspaceID = UInt64(autocompleteItem.workspaceID)
        timeEntry.projectID = UInt64(autocompleteItem.projectID)
        timeEntry.projectGUID = autocompleteItem.projectGUID
        timeEntry.taskID = UInt64(autocompleteItem.taskID)
        timeEntry.projectAndTaskLabel = autocompleteItem.projectAndTaskLabel
        timeEntry.taskLabel = autocompleteItem.taskLabel
        timeEntry.projectLabel = autocompleteItem.projectLabel
        timeEntry.clientLabel = autocompleteItem.clientLabel
        timeEntry.projectColor = autocompleteItem.projectColor

        if timeEntry.isRunning() {
            save(taskID: timeEntry.taskID,
                 projectID: timeEntry.projectID,
                 projectGUID: autocompleteItem.projectGUID,
                 forEntryWithGUID: timeEntry.guid)
        }

        onProjectUpdated?(Project(timeEntry: timeEntry))

        if isNewWorkspace {
            fetchTags()
            updateBillableStatus()
            selectedTags = []
        }
    }

    private func analyticsTrackTimerStart() {
        let actions = actionsUsedBeforeStart.map { $0.rawValue }.reduce(0, { $0 | $1 })
        DesktopLibraryBridge.shared().trackTimerStart(usingActions: TimerEditActionType(rawValue: actions))
        actionsUsedBeforeStart = Set()
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
            self?.timeEntryOnStop()
        }

        let commandContinueObserver = NotificationCenter.default.addObserver(forName: NSNotification.Name(kCommandContinue),
                                                                         object: nil,
                                                                         queue: .main) { [weak self] _ in
            self?.timeEntryContinue()
        }

        let startTimerObserver = NotificationCenter.default.addObserver(forName: NSNotification.Name(kStartTimer),
                                                                        object: nil,
                                                                        queue: .main) { [weak self] _ in
            self?.startStopAction()
        }

        notificationObservers = [displayTimerStateObserver, focusTimerObserver, commandStopObserver, commandContinueObserver, startTimerObserver]
    }

    private func cancelNotificationObservers() {
        notificationObservers.forEach {
            NotificationCenter.default.removeObserver($0)
        }
    }
}

// MARK: - Autocomplete NSTableViewDelegate

// TODO: consider moving into separate file/type
extension TimerViewModel: NSTableViewDelegate {

    func tableView(_ tableView: NSTableView, shouldSelectRow row: Int) -> Bool {
        let autocompleteTable = tableView as! AutoCompleteTable
        autocompleteTable.setCurrentSelected(row, next: true)
        return true
    }

    func tableViewSelectionDidChange(_ notification: Notification) {
        guard let row = descriptionDataSource.input?.autocompleteTableView.lastSelected, row >= 0 else {
            return
        }

        guard let item = descriptionDataSource.item(at: row),
            item.type >= 0 else {
                // category clicked
                return
        }

        fillEntry(fromDescriptionAutocomplete: item)

        descriptionDataSource.input?.resetTable()
        descriptionDataSource.clearFilter()
    }

    func tableView(_ tableView: NSTableView, viewFor tableColumn: NSTableColumn?, row: Int) -> NSView? {
        guard row >= 0, row < descriptionDataSource.filteredOrderedKeys.count else {
            return nil
        }

        guard let item = descriptionDataSource.filteredOrderedKeys.object(at: row) as? AutocompleteItem else {
            return nil
        }
        let autocompleteTable = tableView as! AutoCompleteTable

        let cell = tableView.makeView(withIdentifier: NSUserInterfaceItemIdentifier("AutoCompleteTableCell"),
                                      owner: self) as! AutoCompleteTableCell
        let renderSelected = autocompleteTable.lastSelected != -1 && autocompleteTable.lastSelected == row
        cell.render(item, selected: renderSelected)
        return cell
    }

    func tableView(_ tableView: NSTableView, heightOfRow row: Int) -> CGFloat {
        guard let item = descriptionDataSource.filteredOrderedKeys.object(at: row) as? AutocompleteItem,
            let inputField = descriptionDataSource.input else {
                return 0
        }
        let cellType = AutoCompleteTableCell.cellType(from: item)

        switch cellType {
        case .workspace:
            return inputField.worksapceItemHeight
        default:
            return inputField.itemHeight
        }
    }
}

// MARK: - AutoCompleteViewDataSourceDelegate

extension TimerViewModel: AutoCompleteViewDataSourceDelegate {

    func autoCompleteSelectionDidChange(sender: AutoCompleteViewDataSource, item: Any) {
        if sender == projectDataSource {
            guard let projectItem = item as? ProjectContentItem else {
                return
            }
            fillEntryProject(from: projectItem.item)
            onProjectSelected?(Project(timeEntry: timeEntry))

            actionsUsedBeforeStart.insert(TimerEditActionTypeProject)
        }
    }
}

extension TimerViewModel: TagDataSourceDelegate {

    func tagSelectionChanged(with selectedTags: [Tag]) {
        self.selectedTags = selectedTags.map { $0.name }

        actionsUsedBeforeStart.insert(TimerEditActionTypeTags)
    }
}

extension TimerEditActionType: Hashable {}
