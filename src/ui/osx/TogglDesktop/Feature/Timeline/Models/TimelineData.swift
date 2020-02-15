//
//  TimelineData.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 6/21/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

final class TimelineData {

    enum Section: Int, CaseIterable {
        case timeLabel = 0
        case timeEntry
        case activity
        case background
    }

    struct Constants {
        static let FillEntryGapMinimum: Double = 600.0 // 10 mins
    }

    // MARK: Variables

    private(set) var timeChunks: [TimelineTimestamp] = []
    private(set) var timeEntries: [TimelineBaseTimeEntry]
    private(set) var activities: [TimelineActivity]
    let numberOfSections: Int
    let start: TimeInterval
    let end: TimeInterval
    private(set) var zoomLevel: TimelineDatasource.ZoomLevel
    var isToday: Bool {
        // Get the middle of the day
        let middle = start + (end - start) / 2
        return Calendar.current.isDateInToday(Date(timeIntervalSince1970: middle))
    }

    // MARK: Init

    init(cmd: TimelineDisplayCommand, zoomLevel: TimelineDatasource.ZoomLevel) {
        self.zoomLevel = zoomLevel
        self.start = cmd.start
        self.end = cmd.end
        self.timeEntries = cmd.timeEntries.map { TimelineTimeEntry($0) }.sorted(by: { (lhs, rhs) -> Bool in
            return lhs.start < rhs.start
        })
        numberOfSections = Section.allCases.count
        activities = cmd.activities.compactMap { (activity) -> TimelineActivity? in
            // Map event and sub event
            let events = activity.events.map { event -> TimelineEvent in
                let subEvents = event.subEvents.map { TimelineEvent(title: $0.title, fileName: $0.fileName, duration: $0.duration, durationStr: $0.durationStr, isHeader: $0.isHeader, subEvents: [])}
                return TimelineEvent(title: event.title, fileName: event.fileName, duration: event.duration, durationStr: event.durationStr, isHeader: event.isHeader, subEvents: subEvents)
            }

            // Ignore activity if there is no event inside
            guard !events.isEmpty else { return nil }

            // Get activity
            return TimelineActivity(started: activity.started,
                                    duration: activity.duration,
                                    startTimeStr: activity.startedTimeString,
                                    endTimeStr: activity.endedTimeString,
                                    events: events)
        }
        timeChunks = generateTimelineLabel(for: start,
                                           endDate: end,
                                           zoomLevel: zoomLevel)
        calculateColumnsPositionForTimeline()
    }

    deinit {
        cleanUp()
    }

    // MARK: Public

    func cleanUp() {
        timeChunks.removeAll()
        timeEntries.removeAll()
        activities.removeAll()
    }

    func append(_ entry: TimelineTimeEntry) {
        // Add if need
        let isContains = timeEntries.contains(where: { (item) -> Bool in
            if let timeEntry = item as? TimelineTimeEntry, timeEntry.timeEntry.guid == entry.timeEntry.guid {
                return true
            }
            return false
        })
        if !isContains {
            timeEntries.append(entry)
        }

        // But always reload
        calculateColumnsPositionForTimeline()
    }
    
    func numberOfItems(in section: Int) -> Int {
        guard let section = Section(rawValue: section) else { return 0 }
        switch section {
        case .timeLabel:
            return timeChunks.count
        case .timeEntry:
            return timeEntries.count
        case .activity:
            return activities.count
        case .background:
            return timeChunks.count / 2
        }
    }

    func item(at indexPath: IndexPath) -> Any? {
        guard let section = Section(rawValue: indexPath.section) else { return nil }
        switch section {
        case .timeLabel:
            return timeChunks[safe: indexPath.item]
        case .timeEntry:
            return timeEntries[safe: indexPath.item]
        case .activity:
            return activities[safe: indexPath.item]
        case .background:
            return ""
        }
    }

    func indexPathForItem(with guid: String) -> IndexPath? {
        for (index, item) in timeEntries.enumerated() {
            if let timeEntry = item as? TimelineTimeEntry, timeEntry.timeEntry.guid == guid {
                return IndexPath(item: index, section: Section.timeEntry.rawValue)
            }
        }
        return nil
    }

    func render(with zoomLevel: TimelineDatasource.ZoomLevel) {
        self.zoomLevel = zoomLevel
        timeChunks = generateTimelineLabel(for: start, endDate: end, zoomLevel: zoomLevel)
    }

    func timechunkForItem(at indexPath: IndexPath) -> TimeChunk? {
        guard let section = Section(rawValue: indexPath.section) else {
            return nil
        }
        switch section {
        case .timeLabel:
            return nil
        case .timeEntry:
            return timeEntries[safe: indexPath.item]?.timechunk()
        case .activity:
            return activities[safe: indexPath.item]?.timechunk()
        case .background:
            return nil
        }
    }

    func changeFirstEntryStopTime(at entry: TimelineTimeEntry) {
        let group = getAllConflictedTimeEntries(at: entry)

        // If there is only 1 -> Skip
        guard group.count > 1,
            let firstEntry = group.first,
            let started = entry.timeEntry.started else { return }

        // Set the end time as a start time of selected entry
        DesktopLibraryBridge.shared().updateTimeEntryWithEnd(atTimestamp: started.timeIntervalSince1970 - 1,
                                                             guid: firstEntry.timeEntry.guid)
    }

    func changeLastEntryStartTime(at entry: TimelineTimeEntry) {
        let group = getAllConflictedTimeEntries(at: entry)

        // If there is only 1 -> Skip
        guard group.count > 1,
            let firstEntry = group.first,
            let endAt = firstEntry.timeEntry.ended else { return }

        // Set the start time as a stop time of First entry
        DesktopLibraryBridge.shared().updateTimeEntryWithStart(atTimestamp: endAt.timeIntervalSince1970 + 1,
                                                               guid: entry.timeEntry.guid,
                                                               keepEndTimeFixed: true)
    }

    func continueTimeEntry(_ timeEntry: TimelineTimeEntry) {
        NotificationCenter.default.post(name: NSNotification.Name(kCommandContinue), object: timeEntry.timeEntry.guid)
    }

    func startNewFromEnd(_ timeEntry: TimelineTimeEntry) {
        // Start a running TE if the TE is today
        if timeEntry.isToday() {
            let startTime = timeEntry.end + 1
            guard let guid = DesktopLibraryBridge.shared().starNewTimeEntry(atStarted: 0, ended: 0) else { return }

            // Only set start time if it's not the future
            // Otherwise, the library code gets buggy
            if startTime < Date().timeIntervalSince1970 {
                DesktopLibraryBridge.shared().updateTimeEntryWithStart(atTimestamp: startTime, guid: guid, keepEndTimeFixed: true)
            }
        } else {
            // Create entry and open Editor
            NotificationCenter.default.post(name: Notification.Name(kStarTimeEntryWithStartTime), object: timeEntry.timeEntry.ended)
        }
    }

    func delete(_ timeEntry: TimelineTimeEntry, undoManager: Foundation.UndoManager?) {
        DesktopLibraryBridge.shared().deleteTimeEntryItem(timeEntry.timeEntry, undoManager: undoManager)
    }
}

// MARK: Private

extension TimelineData {

    fileprivate func generateTimelineLabel(for startDate: TimeInterval,
                                                 endDate: TimeInterval,
                                                 zoomLevel: TimelineDatasource.ZoomLevel) -> [TimelineTimestamp] {
        var times: [TimeInterval] = []
        let span = zoomLevel.span
        var current = startDate
        while current <= endDate {
            times.append(current)
            current += span
        }
        return times.map { TimelineTimestamp($0) }.dropLast() // don't render the last hour of next day
    }

    fileprivate func calculateColumnsPositionForTimeline() {
        var calculatedEntries: [TimelineBaseTimeEntry] = []
        var group = -1
        for entry in timeEntries {

            // Check if this time entry intersect with previous one
            // If overlap, increase the number of columns
            var col = 0
            var overlappedTimeEntry: TimelineBaseTimeEntry? = nil
            repeat {

                // Travesal all previous TimeEntry, if it's overlapped -> return
                // O(n) = n
                // It's reasonal since the number of items is small
                overlappedTimeEntry = calculatedEntries.first { timeEntry -> Bool in
                    let isIntersected = entry.isIntersected(with: timeEntry)
                    return isIntersected && (timeEntry.col == col)
                }

                // If overlap
                if overlappedTimeEntry != nil {

                    // Move to next column
                    col += 1
                }

            } while overlappedTimeEntry != nil

            // Group of entry
            // All overlap entries are same group
            if col == 0 {
                group += 1
            }

            // Exit the loop
            entry.setColumn(col)
            entry.setGroup(group)
            calculatedEntries.append(entry)
        }

        // Determine if the TE should has Detail Info
        let firstRowTEs = timeEntries.filter { $0.col == 0 }
        let otherRowTEs = timeEntries.filter { $0.col > 0 }
        for item in firstRowTEs {
            let isOverlaped = otherRowTEs.contains { $0.isIntersected(with: item) }
            item.setHasDetailInfo(!isOverlaped)
        }

        // Add empty time entry
        if let gapEntries = calculateGapEntry(from: firstRowTEs), !gapEntries.isEmpty {
            timeEntries.append(contentsOf: gapEntries)
        }
    }

    private func getAllConflictedTimeEntries(at entry: TimelineTimeEntry) -> [TimelineTimeEntry] {
        // Get all conflicted entry in same group
        return timeEntries
            .filter { $0 is TimelineTimeEntry && $0.group == entry.group }
            .sorted { (lhs, rhs) -> Bool in
                return lhs.col <= rhs.col
            }
            .compactMap { (entry) -> TimelineTimeEntry? in
                return entry as? TimelineTimeEntry
            }
    }

    private func calculateGapEntry(from firstRowEntries: [TimelineBaseTimeEntry]) -> [TimelineBaseTimeEntry]? {
        guard firstRowEntries.count > 1 else { return nil }

        // Sort ASC to make sure it's correct order
        let sortedEntry = firstRowEntries.sorted { (first, second) -> Bool in
            return first.start < second.start
        }

        // Calculate the gap
        var gapEntries: [TimelineBaseTimeEntry] = []
        for (i, item) in sortedEntry.enumerated() {

            // Stop if the index is end
            if i == (sortedEntry.count - 1) {
                break
            }

            // Get the next TE
            let next = sortedEntry[i+1]

            // Make sure the gap is bigger the minimum
            let distance = next.start - item.end
            if abs(distance) >= Constants.FillEntryGapMinimum {
                let start = distance >= 0 ? item.end : next.end
                let end = distance >= 0 ? next.start : item.start
                let gapEntry = TimelineBaseTimeEntry(start: start, end: end, offset: 60.0)
                gapEntries.append(gapEntry)
            }
        }
        return gapEntries
    }
}
