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

    init(start: TimeInterval, end: TimeInterval) {
        self.start = start
        self.end = end
    }

    func timechunk() -> TimeChunk {
        return TimeChunk(start: start, end: end)
    }
}

final class TimelineTimeEntry: TimelineBaseTimeEntry {

    // MARK: Variables

    let timeEntry: TimeEntryViewItem
    let color: NSColor
    private(set) var isOverlap = false

    var isSmall: Bool {
        // It's small bar if duration less than 1 min
        return timeEntry.duration_in_seconds <= 60
    }

    // MARK: Init

    init(_ timeEntry: TimeEntryViewItem) {
        self.timeEntry = timeEntry
        if let color = timeEntry.projectColor, !color.isEmpty {
            self.color = ConvertHexColor.hexCode(toNSColor: timeEntry.projectColor) ?? TimeEntryViewItem.defaultProjectColor()
        } else {
            self.color = TimeEntryViewItem.defaultProjectColor()
        }
        super.init(start: timeEntry.started.timeIntervalSince1970,
                   end: timeEntry.ended.timeIntervalSince1970)
    }

    // MARK: Public

    func setOverlap() {
        isOverlap = true
    }
}
