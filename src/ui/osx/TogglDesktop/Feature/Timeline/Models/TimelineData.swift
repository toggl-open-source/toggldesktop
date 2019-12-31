//
//  TimelineData.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 6/21/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

class TimelineData {

    enum Section: Int, CaseIterable {
        case timeLabel = 0
        case timeEntry
        case activity
    }

    // MARK: Variables

    private(set) var timeChunks: [TimelineTimestamp] = []
    private(set) var timeEntries: [TimelineBaseTimeEntry]
    let activities: [TimelineActivity]
    let numberOfSections: Int
    let start: TimeInterval
    let end: TimeInterval
    private(set) var zoomLevel: TimelineDatasource.ZoomLevel

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

    // MARK: Public

    func numberOfItems(in section: Int) -> Int {
        guard let section = Section(rawValue: section) else { return 0 }
        switch section {
        case .timeLabel:
            return timeChunks.count
        case .timeEntry:
            return timeEntries.count
        case .activity:
            return activities.count
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
        }
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
                                                               guid: entry.timeEntry.guid)
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

                    // Set this TE is not a last column
                    overlappedTimeEntry?.setIsLastColumn(false)
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
        if group >= 0 {
            for i in 0...group {
                let entriesInSameGroup = timeEntries.filter { $0 is TimelineTimeEntry && $0.group == i }
                if entriesInSameGroup.count == 1 {
                    entriesInSameGroup.first?.setHasDetailInfo(true)
                }
            }
        }

        // Add empty time entry
        // Only add if there is a gap between two entries
        if timeEntries.count >= 2 {
            var emptyTimeEntries: [TimelineBaseTimeEntry] = []
            for i in 0..<(timeEntries.count - 1) {
                var current = timeEntries[i]
                let next = timeEntries[i+1]

                // Get the first column
                if let previousCurrent = timeEntries[safe: i - 1], current.end < previousCurrent.end {
                    current = previousCurrent
                }

                if (next.start - current.end) >= 600.0 { // Gap is 10 mins
                    let emptyTimeEntry = TimelineBaseTimeEntry(start: current.end, end: next.start, offset: 60.0)
                    emptyTimeEntries.append(emptyTimeEntry)
                }
            }

            // Add
            timeEntries.append(contentsOf: emptyTimeEntries)
        }
    }

    fileprivate func getAllConflictedTimeEntries(at entry: TimelineTimeEntry) -> [TimelineTimeEntry] {
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
}
