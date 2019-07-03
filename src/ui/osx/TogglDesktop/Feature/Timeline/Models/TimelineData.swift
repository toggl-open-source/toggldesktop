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

    let chunkViews: [TimelineChunkView]
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
        self.chunkViews = cmd.timelineChunks
        self.start = cmd.start
        self.end = cmd.end
        self.timeEntries = cmd.timeEntries.map { TimelineTimeEntry($0) }.sorted(by: { (lhs, rhs) -> Bool in
            return lhs.start < rhs.start
        })
        numberOfSections = Section.allCases.count
        activities = []
        timeChunks = generateTimelineLabel(for: start,
                                           endDate: end,
                                           zoomLevel: zoomLevel)
        generateEmptyTimeEntry()
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
        return times.map { TimelineTimestamp($0) }
    }

    fileprivate func generateEmptyTimeEntry() {
        var firstColumnTimeEntries: [TimelineTimeEntry] = []
        for item in timeEntries {
            guard let timeEntry = item as? TimelineTimeEntry else { continue }
            let start = timeEntry.start
            let end = timeEntry.end

            // Check if it's overlap by comparing the start & end
            let isOverlap = firstColumnTimeEntries.contains { timeEntry -> Bool in
                return (start >= timeEntry.start && start <= timeEntry.end) || (end >= timeEntry.start && end <= timeEntry.end)
            }

            // Set overlap
            timeEntry.isOverlap = isOverlap
            if !isOverlap {
                firstColumnTimeEntries.append(timeEntry)
            }
        }

        // Add empty time entry
        // Only add if there is a gap between two entries
        if firstColumnTimeEntries.count >= 2 {
            var emptyTimeEntries: [TimelineBaseTimeEntry] = []
            for i in 0..<(firstColumnTimeEntries.count - 1) {
                let current = firstColumnTimeEntries[i]
                let next = firstColumnTimeEntries[i+1]

                if (next.start - current.end) >= 600.0 { // Gap is 10 mins
                    let emptyTimeEntry = TimelineBaseTimeEntry(start: current.end, end: next.start, offset: 60.0)
                    emptyTimeEntries.append(emptyTimeEntry)
                }
            }

            // Add
            timeEntries.append(contentsOf: emptyTimeEntries)
        }
    }
}
