//
//  TimelineDashboardViewController.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 6/17/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

protocol TimelineDashboardViewControllerDelegate: class {

    func timelineDidChangeDate(_ date: Date)
}

final class TimelineDashboardViewController: NSViewController {

    // MARK: OUTLET

    @IBOutlet weak var datePickerContainerView: NSView!
    @IBOutlet weak var recordSwitcher: OGSwitch!
    @IBOutlet weak var collectionView: TimelineCollectionView!
    @IBOutlet weak var emptyLbl: NSTextField!
    @IBOutlet weak var emptyActivityLbl: NSTextField!
    @IBOutlet weak var emptyActivityLblPadding: NSLayoutConstraint!
    @IBOutlet weak var zoomContainerView: NSView!
    @IBOutlet weak var collectionViewContainerView: NSScrollView!
    @IBOutlet weak var mainContainerView: NSView!
    @IBOutlet weak var activityRecorderInfoImageView: HoverImageView!
    @IBOutlet weak var activityPanelWidth: NSLayoutConstraint!
    @IBOutlet weak var activityLabelRight: NSLayoutConstraint!
    @IBOutlet weak var permissionBtn: NSButton!

    // MARK: Variables

    weak var delegate: TimelineDashboardViewControllerDelegate?
    private var isOpening = false
    private var selectedGUID: String? {
        didSet {
            if selectedGUID == nil {
                resetHighlightCells()
            }
        }
    }
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
    private lazy var timeEntryHoverPopover: NSPopover = {
        let popover = NSPopover()
        popover.animates = false
        popover.behavior = .semitransient
        popover.contentViewController = timeEntryHoverController
        timeEntryHoverController.popover = popover
        return popover
    }()
    private lazy var activityHoverController: TimelineActivityHoverController = {
        return TimelineActivityHoverController(nibName: "TimelineActivityHoverController", bundle: nil)
    }()
    private lazy var activityHoverPopover: NSPopover = {
        let popover = NSPopover()
        popover.animates = false
        popover.behavior = .semitransient
        popover.contentViewController = activityHoverController
        activityHoverController.popover = popover
        return popover
    }()
    private lazy var editorPopover: EditorPopover = {
        let popover = EditorPopover()
        popover.animates = false
        popover.behavior = .transient
        popover.prepareViewController()
        popover.delegate = self
        return popover
    }()
    private lazy var activityRecorderController: TimelineActivityRecorderViewController = {
        let controller = TimelineActivityRecorderViewController(nibName: "TimelineActivityRecorderViewController", bundle: nil)
        controller.delegate = self
        return controller
    }()
    private lazy var activityRecorderPopover: NoVibrantPopoverView = {
        let popover = NoVibrantPopoverView()
        popover.animates = false
        popover.behavior = .semitransient
        popover.contentViewController = activityRecorderController
        return popover
    }()

    private var isAllPopoverClosed: Bool {
        return !editorPopover.isShown &&
            !activityHoverPopover.isShown &&
            !timeEntryHoverPopover.isShown &&
            !activityRecorderPopover.isShown &&
            !datePickerView.isShown
    }

    // MARK: View
    
    override func viewDidLoad() {
        super.viewDidLoad()

        initCommon()
        initNotifications()
        initCollectionView()
        initTrackingArea()
    }

    deinit {
        NotificationCenter.default.removeObserver(self)
    }

    override func viewDidAppear() {
        super.viewDidAppear()
        isOpening = true

        // Scroll to the first visible item at the first time open Timeline
        if isFirstTime {
            isFirstTime = false
            scrollToVisibleItem()
        }

        // Re-draw the layout when the view did appear
        // because the NSScollerBar appears later -> Mis-position some controls
        collectionView.collectionViewLayout?.invalidateLayout()
    }

    override func viewWillDisappear() {
        super.viewWillDisappear()
        isOpening = false
    }

    func scrollToVisibleItem() {
        datasource.scrollToVisibleItem()
    }

    func updateNextKeyView() {
        guard isViewLoaded else { return }
        recordSwitcher.nextKeyView = datePickerView.previousDateBtn
        datePickerView.updateNextKeyView()
    }

    func render(with cmd: TimelineDisplayCommand) {
        let timeline = TimelineData(cmd: cmd, zoomLevel: zoomLevel)
        let date = Date(timeIntervalSince1970: cmd.start)
        let shouldScroll = datePickerView.currentDate != date
        datePickerView.currentDate = date
        datasource.render(timeline)

        handleEmptyState(timeline)

        if shouldScroll {
            scrollToVisibleItem()
        }

        // After the reload finishes, we hightlight a cell again
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

    @IBAction func permissionBtnOnClicked(_ sender: Any) {
        SystemPermissionManager.shared.grant(.screenRecording)
    }

    override func mouseEntered(with event: NSEvent) {
        super.mouseEntered(with: event)
        zoomContainerView.isHidden = false
    }

    override func mouseExited(with event: NSEvent) {
        super.mouseExited(with: event)
        zoomContainerView.isHidden = true
    }

    func nextDay() {
        datePickerView.nextDateBtnOnTap(self)
    }

    func previousDay() {
        datePickerView.previousDateBtnOnTap(self)
    }
}

// MARK: Private

extension TimelineDashboardViewController {

    fileprivate func initCommon() {
        datasource.delegate = self
        datePickerContainerView.addSubview(datePickerView)
        datePickerView.edgesToSuperView()
        datePickerView.delegate = self
        datePickerView.setBackgroundForTimeline()
        emptyActivityLbl.frameCenterRotation = -90
        activityRecorderInfoImageView.delegate = self
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
        NotificationCenter.default.addObserver(self,
                                               selector: #selector(self.didAddManualTimeNotification),
                                               name: NSNotification.Name.didAdddManualTime,
                                               object: nil)
        NotificationCenter.default.addObserver(self,
                                               selector: #selector(self.editorOnChangeNotification(_:)),
                                               name: Notification.Name(kDisplayTimeEntryEditor),
                                               object: nil)
        NotificationCenter.default.addObserver(self,
                                       selector: #selector(startTimeEntryNoti(_:)),
                                       name: Notification.Name(kStarTimeEntryWithStartTime),
                                       object: nil)
    }

    private func initTrackingArea() {
        let tracking = NSTrackingArea(rect: mainContainerView.bounds,
                                      options: [.mouseEnteredAndExited, .activeAlways, .inVisibleRect],
                                      owner: mainContainerView,
                                      userInfo: nil)
        mainContainerView.addTrackingArea(tracking)
    }

    fileprivate func initCollectionView() {
        collectionView.timelineDelegate = self
    }

    @objc private func handleDisplaySettingNotification(_ noti: Notification) {
        guard let cmd = noti.object as? DisplayCommand,
            let setting = cmd.settings else { return }
        recordSwitcher.setOn(isOn: setting.timeline_recording_enabled, animated: false)
        if setting.timeline_recording_enabled {
            permissionBtn.isHidden = SystemPermissionManager.shared.isGranted(.screenRecording)
        } else {
            permissionBtn.isHidden = true
        }

        updateEmptyActivityText()
    }

    @objc private func reloadTimeline() {
        DesktopLibraryBridge.shared().timelineGetCurrentDate()
    }

    @objc private func didAddManualTimeNotification() {
        // Ignore if the Timeline controller is not showing
        guard view.window != nil else { return }

        // Switch to current date
        DesktopLibraryBridge.shared().timelineSetDate(Date())

        // Get the New item, which the lib just added
        guard let numberOfItem = datasource.timeline?.numberOfItems(in: TimelineData.Section.timeEntry.rawValue) else { return }
        let reversedCollection = (0..<numberOfItem).reversed()
        for index in reversedCollection {
            let indexPath = IndexPath(item: index, section: TimelineData.Section.timeEntry.rawValue)
            let itemView = collectionView.item(at: indexPath)

            // Present editor
            if let item = itemView as? TimelineTimeEntryCell {
                collectionView.scrollToItems(at: Set<IndexPath>(arrayLiteral: indexPath), scrollPosition: .centeredVertically)
                shouldPresentTimeEntryEditor(in: item.popoverView, timeEntry: item.timeEntry.timeEntry, cell: item)
                return
            }
        }
    }

    @objc private func editorOnChangeNotification(_ noti: Notification) {
        guard isOpening,
            let cmd = noti.object as? DisplayCommand,
            let timeEntry = cmd.timeEntry else { return }
        editorPopover.setTimeEntry(timeEntry)
    }

    fileprivate func handleEmptyState(_ timeline: TimelineData) {
        emptyLbl.isHidden = !timeline.timeEntries.isEmpty
        emptyActivityLbl.isHidden = !timeline.activities.isEmpty
        updateEmptyActivityText()
    }

    private func updateEmptyActivityText() {
        emptyActivityLbl.stringValue = recordSwitcher.isOn ? "No activity was recorded yet..." : "Turn on activity\nrecording to see results."
        emptyActivityLblPadding.constant = recordSwitcher.isOn ? -40 : -50
    }

    private func resetHighlightCells() {
        for item in collectionView.visibleItems() {
            if let itemCell = item as? TimelineTimeEntryCell {
                itemCell.isHighlight = false
            }
        }
    }

    private func highlightCells() {
        guard let guid = selectedGUID else { return }
        for item in collectionView.visibleItems() {
            if let itemCell = item as? TimelineTimeEntryCell {
                itemCell.isHighlight = itemCell.timeEntry.timeEntry.guid == guid
            }
        }
    }

    private func closeAllPopovers() {
        let popovers: [NSPopover] = [editorPopover, activityHoverPopover, activityRecorderPopover, timeEntryHoverPopover]
        popovers.forEach { $0.performClose(self) }
    }

    private func closeAllPopoverExceptEditor() {
        let popovers: [NSPopover] = [activityHoverPopover, activityRecorderPopover, timeEntryHoverPopover]
        popovers.forEach { $0.performClose(self) }
    }

    private func getSelectedCell() -> TimelineTimeEntryCell? {
        guard let selectedGUID = selectedGUID else { return nil }
        guard let indexPath = datasource.timeline?.indexPathForItem(with: selectedGUID) else { return nil }
        return collectionView.item(at: indexPath) as? TimelineTimeEntryCell
    }

    @objc private func startTimeEntryNoti(_ noti: Notification) {
        guard let startTime = noti.object as? Date else { return }
        timelineShouldCreateEmptyEntry(with: startTime.timeIntervalSince1970)
    }
}

// MARK: DatePickerViewDelegate

extension TimelineDashboardViewController: DatePickerViewDelegate {

    func datePickerOnChanged(_ sender: DatePickerView, date: Date) {
        editorPopover.close()
        DesktopLibraryBridge.shared().timelineSetDate(date)
        delegate?.timelineDidChangeDate(date)
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
    }

    func datePickerDidTapNextDate(_ sender: DatePickerView) {
        editorPopover.close()
        DesktopLibraryBridge.shared().timelineSetNextDate()
    }

    func datePickerWillOpen(_ sender: DatePickerView) {
        closeAllPopovers()
    }
}

// MARK: TimelineDatasourceDelegate

extension TimelineDashboardViewController: TimelineDatasourceDelegate {

    func shouldPresentTimeEntryHover(in view: NSView, timeEntry: TimelineTimeEntry) {
        guard !editorPopover.isShown else { return }
        timeEntryHoverPopover.show(relativeTo: view.bounds, of: view, preferredEdge: .maxX)
        timeEntryHoverController.render(with: timeEntry)
    }

    func shouldPresentActivityHover(in view: NSView, activity: TimelineActivity) {
        guard !editorPopover.isShown else { return }
        activityHoverPopover.show(relativeTo: view.bounds, of: view, preferredEdge: .maxX)
        activityHoverController.render(activity)
    }

    func shouldPresentTimeEntryEditor(in view: NSView, timeEntry: TimeEntryViewItem, cell: TimelineTimeEntryCell) {
        // Make sure all are closed
        closeAllPopoverExceptEditor()
        resetHighlightCells()

        // Close the Editor if we select the same
        let isSameEntry = timeEntry.guid == selectedGUID
        if isSameEntry {
            selectedGUID = nil
            editorPopover.performClose(self)
            return
        }

        // Or present the Popover
        selectedGUID = timeEntry.guid
        editorPopover.setTimeEntry(timeEntry)
        DesktopLibraryBridge.shared().startEditor(atGUID: timeEntry.guid)

        // Find the cell and present to get the correct position
        // since the toggl_edit causes the Timeline completely reloads -> The cell is reused.
        if let cell = getSelectedCell() {
            cell.isHighlight = true
            let frame = collectionView.convert(cell.popoverView.bounds, from: cell.popoverView)
            editorPopover.show(relativeTo: frame, of: collectionView, preferredEdge: .maxX)
        }
    }

    func startNewTimeEntry(at started: TimeInterval, ended: TimeInterval) {
        guard !editorPopover.isShown else { return }
        guard let guid = DesktopLibraryBridge.shared().starNewTimeEntry(atStarted: started, ended: ended) else { return }
        self.showEditorForTimeEntry(with: guid)
    }

    fileprivate func showEditorForTimeEntry(with guid: String) {
        selectedGUID = guid
        if let cell = getSelectedCell() {
            cell.isHighlight = true
            let frame = collectionView.convert(cell.popoverView.bounds, from: cell.popoverView)
            editorPopover.setTimeEntry(cell.timeEntry.timeEntry)
            editorPopover.show(relativeTo: frame, of: collectionView, preferredEdge: .maxX)
        }
    }

    fileprivate func updatePositionOfEditorIfNeed() {
        guard editorPopover.isShown,
            let cell = getSelectedCell() else { return }
        cell.isHighlight = true
        let frame = collectionView.convert(cell.popoverView.bounds, from: cell.popoverView)
        editorPopover.positioningRect = frame
    }

    func shouldUpdatePanelSize(with activityFrame: CGRect) {
        // Adjust the Activity label to make it center alignment
        activityLabelRight.constant = view.frame.width - activityFrame.origin.x - TimelineFlowLayout.Constants.Divider.SeconDividerRightPadding
    }
}

// MARK: TimelineCollectionViewDelegate

extension TimelineDashboardViewController: TimelineCollectionViewDelegate {

    func timelineShouldCreateEmptyEntry(with startTime: TimeInterval) {
        // Don't create new TE if one of Popover is active
        guard isAllPopoverClosed else {
            closeAllPopovers()
            return
        }

        // Create
        startNewTimeEntry(at: startTime, ended: startTime + 1)
    }
}

extension TimelineDashboardViewController: NSPopoverDelegate {

    func popoverDidClose(_ notification: Notification) {
        guard let popover = notification.object as? NSPopover,
            popover === editorPopover else {
            return
        }
        selectedGUID = nil
    }
}

// MARK: TimelineActivityRecorderViewControllerDelegate

extension TimelineDashboardViewController: TimelineActivityRecorderViewControllerDelegate {

    func timelineActivityRecorderShouldDidClickOnCloseBtn(_ sender: Any) {
        activityRecorderPopover.performClose(self)
    }
}

// MARK: HoverImageViewDelegate

extension TimelineDashboardViewController: HoverImageViewDelegate {

    func hoverImageViewDidMouseExit(_ sender: HoverImageView) {
        guard activityHoverPopover.isShown else { return }
        activityRecorderPopover.performClose(self)
    }

    func hoverImageViewDidMouseEnter(_ sender: HoverImageView) {
        guard !activityHoverPopover.isShown else { return }
        activityRecorderPopover.show(relativeTo: sender.bounds, of: sender, preferredEdge: .minY)
    }
}
