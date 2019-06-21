//
//  TimelineData.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 6/21/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

class TimelineData {

    enum Section: Int {
        case timeLabel = 0
        case timeEntry
        case activity
    }

    // MARK: Variables

    let timeChunks: [TimelineTimeChunk]
    let timeEntries: [TimelineTimeEntry]
    let activities: [TimelineActivity]
    let date: String

    // MARK: Init

    init(_ view: TimelineChunkView, dateLabel: String) {
        date = dateLabel
        timeChunks = []
        timeEntries = []
        activities = []
    }

    // MARK: Public

    func item<T>(at indexPath: IndexPath, type: T.Type) -> T? {
        guard let section = Section(rawValue: indexPath.section) else { return nil }
        switch section {
        case .timeLabel:
            return timeChunks[safe: indexPath.item] as? T
        case .timeEntry:
            return timeEntries[safe: indexPath.item] as? T
        case .activity:
            return activities[safe: indexPath.item] as? T
        }
    }
}
