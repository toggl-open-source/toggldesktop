//
//  TimeEntryUndoPayload.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 12/11/18.
//  Copyright © 2018 Alari. All rights reserved.
//

import Foundation

/// The payload contain the unfo value
final class TimeEntryUndoPayload: Equatable, UndoItemType {

    // MARK: - Variable
    private(set) var descriptionEntry: String?
    private(set) var project: String?
    private(set) var duration: String?
    private(set) var startTime: String?
    private(set) var endTime: String?

    // MARK: - Init
    init(timeEntry: TimeEntryViewItem) {
        self.descriptionEntry = timeEntry.descriptionEntry()
        self.project = timeEntry.projectAndTaskLabel
        self.duration = timeEntry.duration
        self.startTime = timeEntry.startTimeString
        self.endTime = timeEntry.endTimeString
    }

    static func == (lhs: TimeEntryUndoPayload, rhs: TimeEntryUndoPayload) -> Bool {
        return lhs.descriptionEntry == rhs.descriptionEntry &&
                lhs.project == rhs.project &&
                lhs.duration == rhs.duration &&
                lhs.startTime == rhs.startTime &&
                lhs.endTime == rhs.endTime
    }

    func toObjcTimeEntry() -> ObjcTimeEntryPayload {
        return ObjcTimeEntryPayload(descriptionEntry: descriptionEntry,
                                    project: project,
                                    duration: duration,
                                    startTime: startTime,
                                    endTime: endTime)
    }

    func update(with item: Any) {
        guard let item = item as? TimeEntryUndoPayload else { return }
        if descriptionEntry != item.descriptionEntry {
            descriptionEntry = item.descriptionEntry
        }
        if project != item.project {
            project = item.project
        }
        if duration != item.duration {
            duration = item.duration
        }
        if startTime != item.startTime {
            startTime = item.startTime
        }
        if endTime != item.endTime {
            endTime = item.endTime
        }
    }
}

/// Exposed Objc-class, which represent the TimeEntryUndoPayload
@objcMembers final class ObjcTimeEntryPayload: NSObject {

    // MARK: - Variable
    let descriptionEntry: String?
    let project: String?
    let duration: String?
    let startTime: String?
    let endTime: String?

    // MARK: - Init
    init(descriptionEntry: String?, project: String?, duration: String?, startTime: String?, endTime: String?) {
        self.descriptionEntry = descriptionEntry
        self.project = project
        self.duration = duration
        self.startTime = startTime
        self.endTime = endTime
        super.init()
    }
}
