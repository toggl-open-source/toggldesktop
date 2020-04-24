//
//  TimelineDatasource.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 6/21/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

protocol TimelineDatasourceDelegate: class {

    func shouldPresentTimeEntryEditor(in view: NSView, timeEntry: TimeEntryViewItem, cell: TimelineTimeEntryCell)
    func shouldPresentTimeEntryHover(in view: NSView, timeEntry: TimelineTimeEntry)
    func shouldPresentActivityHover(in view: NSView, activity: TimelineActivity)
    func startNewTimeEntry(at started: TimeInterval, ended: TimeInterval)
    func shouldUpdatePanelSize(with activityFrame: CGRect)
    func shouldUpdateEndTime(_ endtime: TimeInterval, for entry: TimelineTimeEntry)
    func shouldUpdateStartTime(_ start: TimeInterval, for entry: TimelineTimeEntry, keepEndTimeFixed: Bool)
    func shouldPresentResizePopover(at cell: TimelineTimeEntryCell, onTopCorner: Bool)
    func shouldHideAllPopover()
    func shouldHandleEmptyState(_ timelineData: TimelineData)
}

final class TimelineDatasource: NSObject {

    private struct Constants {
        static let TimeLabelCellID = NSUserInterfaceItemIdentifier("TimelineTimeLabelCell")
        static let TimeLabelCellXIB = NSNib.Name("TimelineTimeLabelCell")
        static let TimeEntryCellID = NSUserInterfaceItemIdentifier("TimelineTimeEntryCell")
        static let TimeEntryCellXIB = NSNib.Name("TimelineTimeEntryCell")
        static let EmptyTimeEntryCellID = NSUserInterfaceItemIdentifier("TimelineEmptyTimeEntryCell")
        static let EmptyTimeEntryCellXIB = NSNib.Name("TimelineEmptyTimeEntryCell")
        static let ActivityCellID = NSUserInterfaceItemIdentifier("TimelineActivityCell")
        static let ActivityCellXIB = NSNib.Name("TimelineActivityCell")
        static let DividerViewID = NSUserInterfaceItemIdentifier("DividerViewID")
        static let BackgroundViewID = NSUserInterfaceItemIdentifier("TimelineBackgroundCell")
        static let BackgroundViewXIB = NSNib.Name("TimelineBackgroundCell")
    }

    enum ZoomLevel: Int {
        case x1 = 0 // normal
        case x2
        case x3
        case x4

        // The span represent how long between current TimeLabel -> Next label
        var span: TimeInterval {
            switch self {
            case .x4:
                return 7200.0 // Each 2 hours
            case .x1,
                .x2,
                .x3:
                return 3600 // Each 1 hour
            }
        }

        var nextLevel: ZoomLevel? {
            return ZoomLevel(rawValue: self.rawValue + 1)
        }

        var previousLevel: ZoomLevel? {
            return ZoomLevel(rawValue: self.rawValue - 1)
        }

        // If the zoom is x4, two TE could so close
        // This minimum gap prevents it happends
        var minimumGap: CGFloat {
            return 2.0
        }
    }

    // MARK: Variables

    weak var delegate: TimelineDatasourceDelegate?
    private unowned let collectionView: NSCollectionView
    private let flow: TimelineFlowLayout
    private(set) var timeline: TimelineData?
    private var zoomLevel: ZoomLevel = .x1
    private var isUserResizing = false
    private var draggingSession = TimelineDraggingSession()
    private var isUserOnAction: Bool { return isUserResizing || draggingSession.isDragging }
    private var runningTimeEntry: TimelineTimeEntry?
    private var timer: Timer?
    private var cmd: TimelineDisplayCommand?

    // MARK: Init

    init(_ collectionView: NSCollectionView) {
        self.collectionView = collectionView
        self.flow = TimelineFlowLayout()
        super.init()
        flow.flowDelegate = self
        collectionView.delegate = self
        collectionView.dataSource = self
        collectionView.collectionViewLayout = flow
        collectionView.register(NSNib(nibNamed: Constants.TimeLabelCellXIB, bundle: nil), forItemWithIdentifier: Constants.TimeLabelCellID)
        collectionView.register(NSNib(nibNamed: Constants.TimeEntryCellXIB, bundle: nil), forItemWithIdentifier: Constants.TimeEntryCellID)
        collectionView.register(NSNib(nibNamed: Constants.ActivityCellXIB, bundle: nil), forItemWithIdentifier: Constants.ActivityCellID)
        collectionView.register(NSNib(nibNamed: Constants.EmptyTimeEntryCellXIB, bundle: nil), forItemWithIdentifier: Constants.EmptyTimeEntryCellID)
        collectionView.register(TimelineDividerView.self, forSupplementaryViewOfKind: NSCollectionView.elementKindSectionFooter, withIdentifier: Constants.DividerViewID)
        collectionView.register(NSNib(nibNamed: Constants.BackgroundViewXIB, bundle: nil), forItemWithIdentifier: Constants.BackgroundViewID)
        registerForDragAndDrop()
    }

    func render(cmd: TimelineDisplayCommand, zoomLevel: TimelineDatasource.ZoomLevel) {
        self.cmd = cmd
        let data = TimelineData(cmd: cmd,
                                zoomLevel: zoomLevel,
                                runningTimeEntry: getRunningTimeEntryForRendering(start: cmd.start, end: cmd.end))
        render(data)
        delegate?.shouldHandleEmptyState(data)
    }
    
    private func render(_ timeline: TimelineData) {
        // Skip reload if the user is resizing
        guard !isUserOnAction else { return }
        self.timeline?.cleanUp()
        self.timeline = nil
        self.timeline = timeline
        flow.currentDate = Date(timeIntervalSince1970: timeline.start)
        collectionView.reloadData()
    }

    func updateRunningTimeEntry(_ timeEntry: Any?) {
        guard let cmd = cmd, !isUserOnAction else { return }

        // Add
        if let timeEntry = timeEntry as? TimeEntryViewItem, timeEntry.guid != nil {
            guard timeEntry.isRunning() else {
                return
            }

            // Calculate the data again
            // It's essential since it's a straighforward approach to update the running TE
            // If the data is changes (desc, project, color, time, ...)
            runningTimeEntry = TimelineTimeEntry(timeEntry)
            let data = TimelineData(cmd: cmd,
                                    zoomLevel: zoomLevel,
                                    runningTimeEntry: getRunningTimeEntryForRendering(start: cmd.start, end: cmd.end))

            // Render and start the timer
            render(data)
            startTimer()
        } else {
            // Or remove
            runningTimeEntry = nil
            stopTimer()
        }
    }

    func update(_ zoomLevel: ZoomLevel) {
        // Skip reload if the user is resizing
        guard !isUserOnAction else { return }
        self.zoomLevel = zoomLevel
        timeline?.render(with: zoomLevel)
        flow.apply(zoomLevel)
        collectionView.reloadData()
        scrollToVisibleItem()
    }

    func scrollToVisibleItem() {
        guard let timeline = timeline, !isUserOnAction else { return }

        // Get the section should be presented
        // Force render with correct frame then scrolling to desire item
        collectionView.setNeedsDisplay(collectionView.frame)
        collectionView.displayIfNeeded()

        // Scroll to current time if it's today
        if let currentMomentAttribute = flow.currentMomentAttribute,
            timeline.isToday {
            var visiblePoint = currentMomentAttribute.frame.origin
            visiblePoint.y -= (collectionView.enclosingScrollView?.bounds.height ?? 0) / 2 // Middle of the screen
            collectionView.scroll(visiblePoint)
        } else {

            // Skip if both are empty
            if timeline.timeEntries.isEmpty && timeline.activities.isEmpty {
                return
            }

            // Scroll to top Entry or events
            var visibleSection: TimelineData.Section?
            if !timeline.timeEntries.isEmpty {
                visibleSection = .timeEntry
            } else if !timeline.activities.isEmpty {
                visibleSection = .activity
            }
            guard let section = visibleSection else { return }
            collectionView.scrollToItems(at: Set<IndexPath>(arrayLiteral: IndexPath(item: 0, section: section.rawValue)),
                                         scrollPosition: [.centeredHorizontally, .top])
        }
    }

    func timeEntryCell(for guid: String) -> TimelineTimeEntryCell? {
        let cell = collectionView.visibleItems().first { item -> Bool in
            if let cell = item as? TimelineTimeEntryCell,
                cell.timeEntry.timeEntry.guid == guid {
                return true
            }
            return false
        }
        return cell as? TimelineTimeEntryCell
    }
}

extension TimelineDatasource: NSCollectionViewDataSource, NSCollectionViewDelegateFlowLayout, NSCollectionViewDelegate {

    func numberOfSections(in collectionView: NSCollectionView) -> Int {
        guard let timeline = timeline else { return 0 }
        return timeline.numberOfSections
    }

    func collectionView(_ collectionView: NSCollectionView, numberOfItemsInSection section: Int) -> Int {
        guard let timeline = timeline else { return 0 }
        return timeline.numberOfItems(in: section)
    }

    func collectionView(_ collectionView: NSCollectionView, itemForRepresentedObjectAt indexPath: IndexPath) -> NSCollectionViewItem {
        guard let timeline = timeline,
            let section = TimelineData.Section(rawValue: indexPath.section),
            let item = timeline.item(at: indexPath) else { return NSCollectionViewItem() }

        switch section {
        case .timeLabel:
            let cell = collectionView.makeItem(withIdentifier: Constants.TimeLabelCellID, for: indexPath) as! TimelineTimeLabelCell
            let chunk = item as! TimelineTimestamp
            cell.render(chunk)
            return cell
        case .timeEntry:
            switch item {
            case let timeEntry as TimelineTimeEntry:
                let cell = collectionView.makeItem(withIdentifier: Constants.TimeEntryCellID, for: indexPath) as! TimelineTimeEntryCell
                cell.menuDelegate = self
                cell.delegate = self
                cell.config(for: timeEntry)
                return cell
            case let emptyTimeEntry as TimelineBaseTimeEntry:
                let cell = collectionView.makeItem(withIdentifier: Constants.EmptyTimeEntryCellID, for: indexPath) as! TimelineEmptyTimeEntryCell
                cell.config(for: emptyTimeEntry, at: zoomLevel)
                return cell
            default:
                fatalError("We haven't support yet")
            }

        case .activity:
            let cell = collectionView.makeItem(withIdentifier: Constants.ActivityCellID, for: indexPath) as! TimelineActivityCell
            cell.delegate = self
            let activity = item as! TimelineActivity
            cell.config(for: activity)
            return cell
        case .background:
            return collectionView.makeItem(withIdentifier: Constants.BackgroundViewID, for: indexPath)
        }
    }

    func collectionView(_ collectionView: NSCollectionView, viewForSupplementaryElementOfKind kind: NSCollectionView.SupplementaryElementKind, at indexPath: IndexPath) -> NSView {
        guard let section = TimelineData.Section(rawValue: indexPath.section) else { return NSView() }
        if kind == NSCollectionView.elementKindSectionFooter {
            let view = collectionView.makeSupplementaryView(ofKind: kind,
                                                            withIdentifier: Constants.DividerViewID,
                                                            for: indexPath) as! TimelineDividerView
            view.draw(for: section)
            return view
        }
        return NSView()
    }

    func collectionView(_ collectionView: NSCollectionView, didSelectItemsAt indexPaths: Set<IndexPath>) {
        // Skip selection if the dragging
        guard !draggingSession.isDragging else { return }
        guard let indexPath = indexPaths.first,
            let cell = collectionView.item(at: indexPath),
            let item = timeline?.item(at: indexPath) else { return }
        switch item {
        case let timeEntry as TimelineTimeEntry:
            if let cell = cell as? TimelineTimeEntryCell {
                delegate?.shouldPresentTimeEntryEditor(in: cell.popoverView, timeEntry: timeEntry.timeEntry, cell: cell)
                collectionView.deselectItems(at: indexPaths)
            }
        case let item as TimelineBaseTimeEntry:
            delegate?.startNewTimeEntry(at: item.start, ended: item.end)
            collectionView.deselectItems(at: indexPaths)
        default:
            break
        }
    }

    func collectionView(_ collectionView: NSCollectionView, willDisplay item: NSCollectionViewItem, forRepresentedObjectAt indexPath: IndexPath) {
        switch item {
        case let cell as TimelineTimeEntryCell:
            cell.foregroundBox.updateTrackingAreas()
        default:
            break
        }
    }
}

// MARK: TimelineFlowLayoutDelegate

extension TimelineDatasource: TimelineFlowLayoutDelegate {

    func timechunkForItem(at indexPath: IndexPath) -> TimeChunk? {
        return timeline?.timechunkForItem(at: indexPath)
    }

    func isEmptyTimeEntry(at indexPath: IndexPath) -> Bool {
        guard let item = timeline?.item(at: indexPath),
            type(of: item) == TimelineBaseTimeEntry.self else { return false }
        return true
    }

    func columnForItem(at indexPath: IndexPath) -> Int {
        guard let item = timeline?.item(at: indexPath) as? TimelineBaseTimeEntry else { return 0 }
        return item.col
    }

    func shouldDrawDetailBubble(at indexPath: IndexPath) -> Bool {
        guard let item = timeline?.item(at: indexPath) as? TimelineBaseTimeEntry else { return false }
        return item.hasDetailInfo
    }

    func flowLayoutDidUpdateLayout(with activityFrame: CGRect) {
        delegate?.shouldUpdatePanelSize(with: activityFrame)
    }
}

// MARK: TimelineTimeEntryCellDelegate

extension TimelineDatasource: TimelineTimeEntryCellDelegate {

    func timeEntryCellShouldContinue(for entry: TimelineTimeEntry, sender: TimelineTimeEntryCell) {
        timeline?.continueTimeEntry(entry)
    }

    func timeEntryCellShouldStartNew(for entry: TimelineTimeEntry, sender: TimelineTimeEntryCell) {
        timeline?.startNewFromEnd(entry)
    }

    func timeEntryCellShouldDelete(for entry: TimelineTimeEntry, sender: TimelineTimeEntryCell) {
        timeline?.delete(entry, undoManager: collectionView.undoManager)
    }

    func timeEntryCellShouldChangeFirstEntryStopTime(for entry: TimelineTimeEntry, sender: TimelineTimeEntryCell) {
        timeline?.changeFirstEntryStopTime(at: entry)
    }

    func timeEntryCellShouldChangeLastEntryStartTime(for entry: TimelineTimeEntry, sender: TimelineTimeEntryCell) {
        timeline?.changeLastEntryStartTime(at: entry)
    }
}

// MARK: TimelineBaseCellDelegate

extension TimelineDatasource: TimelineBaseCellDelegate {

    func timelineCellMouseDidEntered(_ sender: TimelineBaseCell) {
        switch sender {
        case let timeEntryCell as TimelineTimeEntryCell:
            guard let timeEntry = timeEntryCell.timeEntry else { return }
            delegate?.shouldPresentTimeEntryHover(in: timeEntryCell.popoverView, timeEntry: timeEntry)
        default:
            break
        }
    }

    func timelineCellUpdateEndTime(with event: NSEvent, sender: TimelineBaseCell) {
        isUserResizing = false
        switch sender {
        case let timeEntryCell as TimelineTimeEntryCell:
            guard let timeEntry = timeEntryCell.timeEntry else { return }
            let endAt = convertPointForEndTime(with: event, startTime: timeEntry.start)

            // Update in libray
            delegate?.shouldUpdateEndTime(endAt, for: timeEntry)
        default:
            break
        }
    }

    func timelineCellRedrawEndTime(with event: NSEvent, sender: TimelineBaseCell) {
        isUserResizing = true
        switch sender {
        case let timeEntryCell as TimelineTimeEntryCell:
            guard let timeEntry = timeEntryCell.timeEntry else { return }
            let endAt = convertPointForEndTime(with: event, startTime: timeEntry.start)

            // Update the end time and re-draw
            timeEntry.end = endAt
            flow.invalidateLayout()
            delegate?.shouldPresentResizePopover(at: timeEntryCell, onTopCorner: false)
        default:
            break
        }
    }

    func timelineCellRedrawStartTime(with event: NSEvent, sender: TimelineBaseCell) {
        isUserResizing = true
        switch sender {
        case let timeEntryCell as TimelineTimeEntryCell:
            guard let timeEntry = timeEntryCell.timeEntry else { return }
            let startTime = convertPointForStartTime(with: event, endTime: timeEntry.end)

            // Update and re-draw
            timeEntry.start = startTime
            flow.invalidateLayout()
            delegate?.shouldPresentResizePopover(at: timeEntryCell, onTopCorner: true)
        default:
            break
        }
    }

    func timelineCellUpdateStartTime(with event: NSEvent, sender: TimelineBaseCell) {
        isUserResizing = false
        switch sender {
        case let timeEntryCell as TimelineTimeEntryCell:
            guard let timeEntry = timeEntryCell.timeEntry else { return }
            let startTime = convertPointForStartTime(with: event, endTime: timeEntry.end)

            // Update library
            delegate?.shouldUpdateStartTime(startTime, for: timeEntry, keepEndTimeFixed: true)
        default:
            break
        }
    }

    func timelineCellOpenEditor(_ sender: TimelineBaseCell) {
        if let cell = sender as? TimelineTimeEntryCell {
            delegate?.shouldPresentTimeEntryEditor(in: cell.popoverView, timeEntry: cell.timeEntry.timeEntry, cell: cell)
        }
    }

    private func convertPointForStartTime(with event: NSEvent, endTime: TimeInterval) -> TimeInterval {
        // Convert point to timestamp, depend on the zoom
        let point = collectionView.convert(event.locationInWindow, from: nil)
        let start = flow.convertTimestamp(from: point)

        // Get safe value
        return start < endTime ? start : endTime - 1
    }

    private func convertPointForEndTime(with event: NSEvent, startTime: TimeInterval) -> TimeInterval {
        // Convert point to timestamp, depend on the zoom
        let point = collectionView.convert(event.locationInWindow, from: nil)
        let endedAt = flow.convertTimestamp(from: point)

        // Get safe value
        return endedAt > startTime ? endedAt : startTime + 1
    }
}

// MARK: TimelineActivityCellDelegate

extension TimelineDatasource: TimelineActivityCellDelegate {

    func timelineActivityPresentPopover(_ sender: TimelineActivityCell) {
        guard let activity = sender.activity else { return }
        delegate?.shouldPresentActivityHover(in: sender.view, activity: activity)
    }
}

// MARK: Private

extension TimelineDatasource {

    private func registerForDragAndDrop() {
        collectionView.registerForDraggedTypes([NSPasteboard.PasteboardType.string])
        collectionView.setDraggingSourceOperationMask(NSDragOperation.move, forLocal: true)
    }

    private func startTimer() {
        stopTimer()
        timer = Timer.scheduledTimer(timeInterval: 10, target: self, selector: #selector(self.timerUpdateOnTick), userInfo: nil, repeats: true)
    }

    private func stopTimer() {
        timer?.invalidate()
        timer = nil
    }

    @objc private func timerUpdateOnTick() {
        guard runningTimeEntry != nil else {
            stopTimer()
            return
        }
        runningTimeEntry?.updateEndTimeForRunning()
        flow.invalidateLayout()
        print("---------------------------- Timer Tick")
    }

    private func getRunningTimeEntryForRendering(start: TimeInterval, end: TimeInterval) -> TimeEntryViewItem? {
        let middle = start + (end - start) / 2
        if Calendar.current.isDateInToday(Date(timeIntervalSince1970: middle)) {
            return runningTimeEntry?.timeEntry
        }
        return nil
    }
}

// MARK: Drag and Drop

extension TimelineDatasource {

    func collectionView(_ collectionView: NSCollectionView, canDragItemsAt indexPaths: Set<IndexPath>, with event: NSEvent) -> Bool {
        guard let firstIndex = indexPaths.first,
            let draggedCell = collectionView.item(at: firstIndex) else { return false }
        
        // Only accept drag on Timeline Entry
        switch draggedCell {
        case let cell as TimelineTimeEntryCell:
            guard let timeEntry = cell.timeEntry else { return false }

            // Calculate the distance between the mouse position on the selected TE and the Start Time
            // It's essential to update the time later
            let point = collectionView.convert(event.locationInWindow, from: nil)
            let mouseTimestamp = flow.convertTimestamp(from: point)
            draggingSession.mouseDistanceFromTop = abs(mouseTimestamp - timeEntry.start)
            return true
        default:
            return false
        }
    }

    func collectionView(_ collectionView: NSCollectionView, pasteboardWriterForItemAt indexPath: IndexPath) -> NSPasteboardWriting? {
        let data = NSKeyedArchiver.archivedData(withRootObject: Array(indexPath))
        let pbItem = NSPasteboardItem()
        pbItem.setData(data, forType: NSPasteboard.PasteboardType.string)
        return pbItem
    }

    func collectionView(_ collectionView: NSCollectionView, draggingSession session: NSDraggingSession, willBeginAt screenPoint: NSPoint, forItemsAt indexPaths: Set<IndexPath>) {
        draggingSession.indexPath = indexPaths.first
        delegate?.shouldHideAllPopover()
    }

    func collectionView(_ collectionView: NSCollectionView, validateDrop draggingInfo: NSDraggingInfo, proposedIndexPath proposedDropIndexPath: AutoreleasingUnsafeMutablePointer<NSIndexPath>, dropOperation proposedDropOperation: UnsafeMutablePointer<NSCollectionView.DropOperation>) -> NSDragOperation {

        // Local position of the dragging mouse
        let position = collectionView.convert(draggingInfo.draggingLocation, from: nil)

        // Only allow to drop on the TimeEntry section
        guard flow.isInTimeEntrySection(at: position) else {
            return []
        }

        return .copy
    }

    func collectionView(_ collectionView: NSCollectionView, acceptDrop draggingInfo: NSDraggingInfo, indexPath: IndexPath, dropOperation: NSCollectionView.DropOperation) -> Bool {

        // Verify that the drop position is in the Timeline Entry Section
        let position = collectionView.convert(draggingInfo.draggingLocation, from: nil)
        guard flow.isInTimeEntrySection(at: position) else { return false }

        // Get all things
        guard let indexPath = draggingSession.indexPath,
            let cell = collectionView.item(at: indexPath) as? TimelineTimeEntryCell,
            let timeEntry = cell.timeEntry else {
                return false
        }

        // Update position
        let delta = draggingSession.mouseDistanceFromTop ?? 0.0
        let mouseTimestamp = flow.convertTimestamp(from: position)
        let droppedStartTime = abs(mouseTimestamp - delta)

        // Update UI
        let duration = timeEntry.end - timeEntry.start
        timeEntry.start = droppedStartTime
        timeEntry.end = timeEntry.start + duration
        flow.invalidateLayout()

        // Save
        draggingSession.acceptDrop(at: droppedStartTime, for: timeEntry)
        return true
    }

    func collectionView(_ collectionView: NSCollectionView, draggingSession session: NSDraggingSession, endedAt screenPoint: NSPoint, dragOperation operation: NSDragOperation) {
        let startTime = draggingSession.finalStartTime
        let timeEntry = draggingSession.item

        // Reset all
        draggingSession.reset()

        // Update lib
        if let startTime = startTime, let timeEntry = timeEntry {
            delegate?.shouldUpdateStartTime(startTime, for: timeEntry, keepEndTimeFixed: false)
        }
    }
}
