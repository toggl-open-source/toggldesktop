//
//  TimelineTimeEntry.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 6/21/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

/// Represent the base Timeline Time Entry
class TimelineBaseTimeEntry {

    struct Constant {
        static let MinimumSecondOverlap: Double = 60 // seconds
    }

    // MARK: Variables

    var start: TimeInterval
    var end: TimeInterval

    // Group of overlap entries -> Help to resolve the overlap later
    private(set) var group: Int = -1

    // The number column of the Timeline Entry. The first col is 0
    private(set) var col: Int = 0

    /// Determine if this TimeEntry is overlap with another TEs
    var isOverlap: Bool {
        return col > 0
    }

    /// Some TEs has a detail bubble
    private(set) var hasDetailInfo = false

    // MARK: Init

    /// Init from given time range
    /// - Parameters:
    ///   - start: The start timestamp
    ///   - end: The start timestamp
    ///   - offset: The offset, which is depends on the Zoom level
    init(start: TimeInterval, end: TimeInterval, offset: TimeInterval = 0) {
        self.start = start + offset
        self.end = end - offset
    }

    /// Get a time chunck, which consists of the Start and End timestamp
    /// - Returns: TimeChunk
    func timechunk() -> TimeChunk {
        return TimeChunk(start: start, end: end)
    }

    /// Check whether or not this TimeEntry intersects with the given Time Entry
    /// - Parameter entry: Test Time Entry
    /// - Returns: isIntersect
    func isIntersected(with entry: TimelineBaseTimeEntry) -> Bool {

        // Skip overlap if the diff is less than 60s
        if abs(entry.start - end) <= Constant.MinimumSecondOverlap ||
            abs(entry.end - start) <= Constant.MinimumSecondOverlap {
            return false
        }

        // Convert to rect to easier check intersects
        let currentRect = CGRect(x: 1, y: start, width: 1, height: abs(end - start))
        let entryRect = CGRect(x: 1, y: entry.start, width: 1, height: abs(entry.end - entry.start))
        return currentRect.intersects(entryRect)
    }

    // MARK: Setter

    func setGroup(_ group: Int) {
        self.group = group
    }

    func setColumn(_ col: Int) {
        self.col = col
    }

    func setHasDetailInfo(_ hasDetailInfo: Bool) {
        self.hasDetailInfo = hasDetailInfo
    }
}

/// Main Timeline Time Entry
final class TimelineTimeEntry: TimelineBaseTimeEntry {

    // MARK: Variables

    let timeEntry: TimeEntryViewItem
    let color: NSColor
    let name: String

    /// It's required from the design
    /// Small Time Entry has some different configs
    var isSmall: Bool {
        // It's small bar if duration less than 1 min
        return abs(start - end) <= 60
    }

    // MARK: Init

    init(_ timeEntry: TimeEntryViewItem) {
        self.timeEntry = timeEntry
        self.name = timeEntry.descriptionName

        // Get the color from TimeEntryViewItem or fallback to the default
        if let color = timeEntry.projectColor, !color.isEmpty {
            self.color = ConvertHexColor.hexCode(toNSColor: timeEntry.projectColor) ?? TimeEntryViewItem.defaultProjectColor()
        } else {
            self.color = TimeEntryViewItem.defaultProjectColor()
        }
        super.init(start: timeEntry.started.timeIntervalSince1970,
                   end: timeEntry.ended.timeIntervalSince1970)
    }

    // MARK: Public

    func isToday() -> Bool {
        let date = Date(timeIntervalSince1970: end)
        return Calendar.current.isDateInToday(date)
    }

    func updateEndTimeForRunning() {
        end = Date().timeIntervalSince1970
    }
}

/// Simple item to represent simple placeholder view in place where the time entry will be placed later.
/// For example, used to show a view when user is creating entry with dragging action.
final class TimelineTimeEntryPlaceholder: TimelineBaseTimeEntry {
}
