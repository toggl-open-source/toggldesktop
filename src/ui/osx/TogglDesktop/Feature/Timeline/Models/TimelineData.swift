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
    private(set) var timeChunks: [TimelineTimeChunk] = []
    let timeEntries: [TimelineTimeEntry]
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

    func timestampForItem(at indexPath: IndexPath) -> Timestamp? {
        guard let section = Section(rawValue: indexPath.section) else {
            return nil
        }
        switch section {
        case .timeLabel:
            return nil
        case .timeEntry:
            return timeEntries[safe: indexPath.item]?.timestamp()
        case .activity:
            return activities[safe: indexPath.item]?.timestamp()
        }
    }
}

// MARK: Private

extension TimelineData {

    fileprivate func generateTimelineLabel(for startDate: TimeInterval,
                                                 endDate: TimeInterval,
                                                 zoomLevel: TimelineDatasource.ZoomLevel) -> [TimelineTimeChunk] {
        var times: [TimeInterval] = []
        let timeGap = zoomLevel.timeGap
        var current = startDate
        while current <= endDate {
            times.append(current)
            current += timeGap
        }
        return times.map { TimelineTimeChunk($0) }
    }
}
