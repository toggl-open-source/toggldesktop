//
//  TimerViewModel.swift
//  TogglDesktop
//
//  Created by Andrew Nester on 28.07.2020.
//  Copyright © 2020 Toggl. All rights reserved.
//

import Foundation

final class TimerViewModel: NSObject {

    var entryDescription: String = "" {
        didSet {
            guard entryDescription != oldValue else { return }
            timeEntry.entryDescription = entryDescription
            updateAutocomplete()
            onDescriptionChanged?(entryDescription)
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

    var onIsRunning: ((Bool) -> Void)?
    var onDescriptionChanged: ((String) -> Void)?
    var onDurationChanged: ((String) -> Void)?
    var onTagSelected: ((Bool) -> Void)?
    var onProjectSelected: ((Project?) -> Void)?
    var onDescriptionFocusChanged: ((Bool) -> Void)?
    var onTouchBarUpdateRunningItem: ((TimeEntryViewItem) -> Void)?

    /// Timer tick notification will be posted every second if there is a running time entry.
    static let timerOnTickNotification = NSNotification.Name("TimerForRunningTimeEntryOnTicket")

    private var timeEntry = TimeEntryViewItem()
    private var notificationObservers: [AnyObject] = []
    private var autocompleteDataSource = LiteAutoCompleteDataSource(notificationName: kDisplayMinitimerAutocomplete)
    private var timer: Timer!

    override init() {
        super.init()
        setupNotificationObservers()

        timer = Timer.scheduledTimer(timeInterval: 1.0,
                                     target: self,
                                     selector: #selector(timerFired(_:)),
                                     userInfo: nil,
                                     repeats: true)
    }

    deinit {
        cancelNotificationObservers()
        timer.invalidate()
    }

    func start() {
        if timeEntry.isRunning() {
            NotificationCenter.default.post(name: NSNotification.Name(rawValue: kCommandStop), object: nil, userInfo: nil)
        } else {
            NotificationCenter.default.post(name: NSNotification.Name(rawValue: kCommandNew), object: timeEntry, userInfo: nil)
            onDescriptionFocusChanged?(false)

            autocompleteDataSource.input?.resetTable()
            autocompleteDataSource.clearFilter()
        }
    }

    func setAutocompleteInput(_ input: BetterFocusAutoCompleteInput) {
        autocompleteDataSource.setFilter("")
        autocompleteDataSource.input = input
        input.autocompleteTableView.delegate = self
    }

    func selectAutocompleteItem(at index: Int) -> Bool {
        guard let item = autocompleteDataSource.item(at: index) else {
            return false
        }
        fillEntry(from: item)

        autocompleteDataSource.input?.resetTable()
        autocompleteDataSource.clearFilter()

        return true
    }

    // MARK: - Other

    @objc
    private func timerFired(_ timer: Timer) {
        guard timeEntry.isRunning() else { return }

        let formattedDuration = DesktopLibraryBridge.shared().convertDuraton(inSecond: timeEntry.duration_in_seconds)
        durationString = formattedDuration

        NotificationCenter.default.post(name: Self.timerOnTickNotification, object: nil)
    }

    private func updateAutocomplete() {
        autocompleteDataSource.setFilter(entryDescription)
        autocompleteDataSource.input?.autocompleteTableView.resetSelected()
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
            self.entryDescription = description
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
        entryDescription = ""
        durationString = ""
        focusTimer()

        timeEntry = TimeEntryViewItem()

        onTagSelected?(false)
        onProjectSelected?(nil)
    }

    private func fillEntry(from autocompleteItem: AutocompleteItem) {
        // User has selected a autocomplete item.
        // It could be a time entry, a task or a project.
        timeEntry.workspaceID = UInt64(autocompleteItem.workspaceID)
        timeEntry.projectID = UInt64(autocompleteItem.projectID)
        timeEntry.taskID = UInt64(autocompleteItem.taskID)
        timeEntry.projectAndTaskLabel = autocompleteItem.projectAndTaskLabel
        timeEntry.taskLabel = autocompleteItem.taskLabel
        timeEntry.projectLabel = autocompleteItem.projectLabel
        timeEntry.clientLabel = autocompleteItem.clientLabel
        timeEntry.projectColor = autocompleteItem.projectColor
        timeEntry.tags = autocompleteItem.tags
        timeEntry.billable = autocompleteItem.billable

        if let newDescription = autocompleteItem.description {
            timeEntry.entryDescription = newDescription.isEmpty ? autocompleteItem.taskLabel : newDescription
        }
        entryDescription = timeEntry.entryDescription

        onTagSelected?(timeEntry.tags?.isEmpty == false)

        let hasProject = timeEntry.projectLabel != nil && timeEntry.projectLabel.isEmpty == false
        if hasProject {
            let project = Project(timeEntry: timeEntry)
            onProjectSelected?(project)
        } else {
            onProjectSelected?(nil)
        }

        focusTimer()
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
        guard let row = autocompleteDataSource.input?.autocompleteTableView.selectedRow, row >= 0 else {
            return
        }

        guard let item = autocompleteDataSource.item(at: row),
            item.type >= 0 else {
                // category clicked
                return
        }

        fillEntry(from: item)

        autocompleteDataSource.input?.resetTable()
        autocompleteDataSource.clearFilter()
    }

    func tableView(_ tableView: NSTableView, viewFor tableColumn: NSTableColumn?, row: Int) -> NSView? {
        guard row >= 0, row < autocompleteDataSource.filteredOrderedKeys.count else {
            return nil
        }

        guard let item = autocompleteDataSource.filteredOrderedKeys.object(at: row) as? AutocompleteItem else {
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
        guard let item = autocompleteDataSource.filteredOrderedKeys.object(at: row) as? AutocompleteItem,
            let inputField = autocompleteDataSource.input else {
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

// MARK: - Project struct

extension TimerViewModel {

    struct Project {
        let color: NSColor
        let attributedTitle: NSAttributedString

        init(timeEntry: TimeEntryViewItem) {
            self.color = ConvertHexColor.hexCode(toNSColor: timeEntry.projectColor)
            self.attributedTitle = ProjectTitleFactory().title(for: timeEntry)
        }
    }
}
