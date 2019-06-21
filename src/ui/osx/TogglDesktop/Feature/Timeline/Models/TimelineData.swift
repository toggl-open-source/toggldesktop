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
    let timeChunks: [TimelineTimeChunk]
    let timeEntries: [TimelineTimeEntry]
    let activities: [TimelineActivity]
    let date: String
    let numberOfSections: Int

    // MARK: Init

    init(_ chunkViews: [TimelineChunkView], timeEntries: [TimeEntryViewItem], dateLabel: String) {
        self.chunkViews = chunkViews
        numberOfSections = Section.allCases.count
        date = dateLabel
        timeChunks = []
        self.timeEntries = timeEntries.map { TimelineTimeEntry($0) }
        activities = []
    }

    convenience init(cmd: TimelineDisplayCommand) {
        self.init(cmd.timelineChunks, timeEntries: cmd.timeEntries, dateLabel: cmd.timelineDate)
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
        switch zoomLevel {
        case .x4:
            break
        case .x1,
             .x2,
             .x3:
            return
        }
    }
}
