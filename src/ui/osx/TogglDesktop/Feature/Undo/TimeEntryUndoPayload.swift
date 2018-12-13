//
//  TimeEntryUndoPayload.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 12/11/18.
//  Copyright © 2018 Alari. All rights reserved.
//

import Foundation

/// The payload contain the unfo value
final class TimeEntryUndoPayload: Equatable {

    // MARK: - Variable
    let descriptionEntry: String
    let project: String

    // MARK: - Init
    init(descriptionEntry: String, project: String) {
        self.descriptionEntry = descriptionEntry
        self.project = project
    }

    static func == (lhs: TimeEntryUndoPayload, rhs: TimeEntryUndoPayload) -> Bool {
        return lhs.descriptionEntry == rhs.descriptionEntry &&
                lhs.project == rhs.project
    }

    func toObjcTimeEntry() -> ObjcTimeEntryPayload {
        return ObjcTimeEntryPayload(descriptionEntry: descriptionEntry, project: project)
    }
}

/// Exposed Objc-class, which represent the TimeEntryUndoPayload
@objcMembers final class ObjcTimeEntryPayload: NSObject {

    // MARK: - Variable
    let descriptionEntry: String
    let project: String

    // MARK: - Init
    init(descriptionEntry: String, project: String) {
        self.descriptionEntry = descriptionEntry
        self.project = project
        super.init()
    }

    override var description: String {
        return "descriptionEntry \(descriptionEntry), project \(project)"
    }
}
