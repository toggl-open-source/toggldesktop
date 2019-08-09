//
//  TimelineDashboardViewController.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 6/17/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

final class TimelineDashboardViewController: NSViewController {

    // MARK: OUTLET

    @IBOutlet weak var datePickerContainerView: NSView!
    @IBOutlet weak var recordSwitcher: OGSwitch!
    @IBOutlet weak var collectionView: NSCollectionView!
    @IBOutlet weak var emptyLbl: NSTextField!
    
    // MARK: Variables

    private var selectedGUID: String?
    private var isFirstTime = true
    lazy var datePickerView: DatePickerView = DatePickerView.xibView()
    private lazy var datasource = TimelineDatasource(collectionView)
    private var zoomLevel: TimelineDatasource.ZoomLevel = .x1 {
        didSet {
            datasource.update(zoomLevel)
        }
    }
    private lazy var timeEntryHoverController: TimelineTimeEntryHoverViewController = {
        return TimelineTimeEntryHoverViewController(nibName: "TimelineTimeEntryHoverViewController", bundle: nil)
    }()
    private lazy var hoverPopover: NSPopover = {
        let popover = NSPopover()
        popover.animates = false
        popover.behavior = .semitransient
        popover.contentViewController = timeEntryHoverController
        timeEntryHoverController.popover = popover
        return popover
    }()
    private lazy var editorPopover: EditorPopover = {
        let popover = EditorPopover()
        popover.animates = false
        popover.behavior = .transient
        popover.prepareViewController()
        return popover
    }()

    // MARK: View
    
    override func viewDidLoad() {
        super.viewDidLoad()

        initCommon()
        initNotifications()
        initCollectionView()
    }

    deinit {
        NotificationCenter.default.removeObserver(self)
    }

    override func viewDidAppear() {
        super.viewDidAppear()

        // Scroll to the first visible item at the first time open Timeline
        if isFirstTime {
            isFirstTime = false
            datasource.scrollToVisibleItem()
        }
    }

    func updateNextKeyView() {
        recordSwitcher.nextKeyView = datePickerView.previousDateBtn
        datePickerView.updateNextKeyView()
    }

    func render(with cmd: TimelineDisplayCommand) {
        let timeline = TimelineData(cmd: cmd, zoomLevel: zoomLevel)
        let date = Date(timeIntervalSince1970: cmd.start)
        datePickerView.currentDate = date
        datasource.render(timeline)
        emptyLbl.isHidden = !timeline.timeEntries.isEmpty
        updatePositionOfEditorIfNeed()
    }
    
    @IBAction func recordSwitchOnChanged(_ sender: Any) {
        DesktopLibraryBridge.shared().enableTimelineRecord(recordSwitcher.isOn)
    }

    @IBAction func zoomLevelDecreaseOnChange(_ sender: Any) {
        guard let next = zoomLevel.nextLevel else { return }
        zoomLevel = next
    }

    @IBAction func zoomLevelIncreaseOnChange(_ sender: Any) {
        guard let previous = zoomLevel.previousLevel else { return }
        zoomLevel = previous
    }
}

// MARK: Private

extension TimelineDashboardViewController {

    fileprivate func initCommon() {
        datasource.delegate = self
        datePickerContainerView.addSubview(datePickerView)
        datePickerView.edgesToSuperView()
        datePickerView.delegate = self
    }

    fileprivate func initNotifications() {
        NotificationCenter.default.addObserver(self,
                                               selector: #selector(self.handleDisplaySettingNotification(_:)),
                                               name: NSNotification.Name(kDisplaySettings),
                                               object: nil)
        NotificationCenter.default.addObserver(self,
                                               selector: #selector(self.reloadTimeline),
                                               name: Notification.Name(kDisplayTimeEntryList),
                                               object: nil)
    }

    fileprivate func initCollectionView() {

    }

    @objc private func handleDisplaySettingNotification(_ noti: Notification) {
        guard let cmd = noti.object as? DisplayCommand,
            let setting = cmd.settings else { return }
        recordSwitcher.setOn(isOn: setting.timeline_recording_enabled, animated: false)
    }

    @objc private func reloadTimeline() {
        DesktopLibraryBridge.shared().timelineGetCurrentDate()
    }
}

// MARK: DatePickerViewDelegate

extension TimelineDashboardViewController: DatePickerViewDelegate {

    func datePickerOnChanged(_ sender: DatePickerView, date: Date) {
        editorPopover.close()
        DesktopLibraryBridge.shared().timelineSetDate(date)
        datasource.scrollToVisibleItem()
    }

    func datePickerShouldClose(_ sender: DatePickerView) {

    }

    func isTimeEntryRunning(_ sender: DatePickerView) -> Bool {
        return false
    }

    func shouldOpenCalendar(_ sender: DatePickerView) -> Bool {
        return true
    }

    func datePickerDidTapPreviousDate(_ sender: DatePickerView) {
        editorPopover.close()
        DesktopLibraryBridge.shared().timelineSetPreviousDate()
        datasource.scrollToVisibleItem()
    }

    func datePickerDidTapNextDate(_ sender: DatePickerView) {
        editorPopover.close()
        DesktopLibraryBridge.shared().timelineSetNextDate()
        datasource.scrollToVisibleItem()
    }
}

// MARK: TimelineDatasourceDelegate

extension TimelineDashboardViewController: TimelineDatasourceDelegate {

    func shouldPresentTimeEntryHover(in view: NSView, timeEntry: TimelineTimeEntry) {
        // Dont' show hover if the user is editing
        guard !editorPopover.isShown else { return }
        hoverPopover.show(relativeTo: view.bounds, of: view, preferredEdge: .maxX)
        timeEntryHoverController.render(with: timeEntry)
    }

    func shouldDismissTimeEntryHover() {
        hoverPopover.close()
    }

    func shouldPresentTimeEntryEditor(in view: NSView, timeEntry: TimeEntryViewItem) {
        shouldDismissTimeEntryHover()
        selectedGUID = timeEntry.guid
        editorPopover.show(relativeTo: view.bounds, of: view, preferredEdge: .maxX)
        editorPopover.setTimeEntry(timeEntry)
    }

    func startNewTimeEntry(at started: TimeInterval, ended: TimeInterval) {
        guard let guid = DesktopLibraryBridge.shared().starNewTimeEntry(atStarted: started, ended: ended) else { return }
        self.showEditorForTimeEntry(with: guid)
    }

    fileprivate func showEditorForTimeEntry(with guid: String) {
        guard let timeEntries = datasource.timeline?.timeEntries else { return }

        // Get the index for item with guid
        let foundIndex = timeEntries.firstIndex(where: { entry -> Bool in
            if let timeEntry = entry as? TimelineTimeEntry,
                let timeEntryGUID = timeEntry.timeEntry.guid,
                timeEntryGUID == guid {
                return true
            }
            return false
        })

        // Get all essential data to present Editor
        guard let index = foundIndex else { return }
        let indexPath = IndexPath(item:index, section: TimelineData.Section.timeEntry.rawValue)
        guard let item = datasource.timeline?.item(at: indexPath) as? TimelineTimeEntry,
            let cell = collectionView.item(at: indexPath) else { return }
        shouldPresentTimeEntryEditor(in: cell.view, timeEntry: item.timeEntry)
    }

    fileprivate func updatePositionOfEditorIfNeed() {
        guard editorPopover.isShown,
            let selectedGUID = selectedGUID else { return }

        // Search the last index for GUID
        // Since everytime we change the Time entry data
        // New Entry will come from Library
        let index = datasource.timeline?.timeEntries.firstIndex(where: { (entry) -> Bool in
            if let timeEntry = entry as? TimelineTimeEntry {
                return timeEntry.timeEntry.guid == selectedGUID
            }
            return false
        })
        guard let item = index else {
            editorPopover.close()
            return
        }
        let indexPath = IndexPath(item: item, section: TimelineData.Section.timeEntry.rawValue)
        guard let cell = collectionView.item(at: indexPath) else {
            editorPopover.close()
            return
        }

        editorPopover.animates = false
        editorPopover.show(relativeTo: cell.view.bounds, of: cell.view, preferredEdge: .maxX)
    }
}
