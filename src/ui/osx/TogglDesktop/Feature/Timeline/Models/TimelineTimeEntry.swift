//
//  TimelineTimeEntry.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 6/21/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

class TimelineBaseTimeEntry {

    let start: TimeInterval
    let end: TimeInterval
    private(set) var col: Int = 0
    var isOverlap: Bool {
        return col > 0
    }

    init(start: TimeInterval, end: TimeInterval, offset: TimeInterval = 0) {
        self.start = start + offset
        self.end = end - offset
    }

    func timechunk() -> TimeChunk {
        return TimeChunk(start: start, end: end)
    }

    func update(_ col: Int) {
        self.col = col
    }

    func isIntersected(with entry: TimelineBaseTimeEntry) -> Bool {
        return (start >= entry.start && start <= entry.end)
            || (end >= entry.start && end <= entry.end)
            || (start >= entry.start && end <= entry.end)
    }
}

final class TimelineTimeEntry: TimelineBaseTimeEntry {

    // MARK: Variables

    let timeEntry: TimeEntryViewItem
    let color: NSColor
    let name: String

    var isSmall: Bool {
        // It's small bar if duration less than 1 min
        return timeEntry.duration_in_seconds <= 60
    }

    // MARK: Init

    init(_ timeEntry: TimeEntryViewItem) {
        self.timeEntry = timeEntry
        self.name = timeEntry.descriptionName
        if let color = timeEntry.projectColor, !color.isEmpty {
            self.color = ConvertHexColor.hexCode(toNSColor: timeEntry.projectColor) ?? TimeEntryViewItem.defaultProjectColor()
        } else {
            self.color = TimeEntryViewItem.defaultProjectColor()
        }
        super.init(start: timeEntry.started.timeIntervalSince1970,
                   end: timeEntry.ended.timeIntervalSince1970)
    }
}
