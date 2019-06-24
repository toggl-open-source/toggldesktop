//
//  TimelineTimeEntry.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 6/21/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

final class TimelineTimeEntry {

    // MARK: Variables

    let timeEntry: TimeEntryViewItem

    // MARK: Init

    init(_ timeEntry: TimeEntryViewItem) {
        self.timeEntry = timeEntry
    }

    func timestamp() -> Timestamp {
        return Timestamp(start: timeEntry.started.timeIntervalSince1970,
                         end: timeEntry.ended.timeIntervalSince1970)
    }
}
