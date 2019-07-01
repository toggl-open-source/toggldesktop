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
    let timeEntries: [TimelineBaseTimeEntry]
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
        self.timeEntries = cmd.timeEntries.map { TimelineTimeEntry($0) }
        numberOfSections = Section.allCases.count
        activities = []
        timeChunks = generateTimelineLabel(for: start,
                                           endDate: end,
                                           zoomLevel: zoomLevel)
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
}
