//
//  TimeEntrySnapshot.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 12/11/18.
//  Copyright © 2018 Alari. All rights reserved.
//

import Foundation

/// The payload contain the unfo value
final class TimeEntrySnapshot {

    // MARK: - Variable
    private let descriptionEntryStack: UndoStack<String>
    private let projectNameStack: UndoStack<String>
    private let durationStack: UndoStack<String>
    private let startTimeStack: UndoStack<String>
    private let endTimeStack: UndoStack<String>

    // MARK: - Init
    init(timeEntry: TimeEntryViewItem, levelOfUndo: Int) {
        self.descriptionEntryStack = UndoStack<String>(firstItem: timeEntry.descriptionEntry().safeUnwrapped,
													   maxCount: levelOfUndo)
        self.projectNameStack = UndoStack<String>(firstItem: timeEntry.projectAndTaskLabel.safeUnwrapped,
												  maxCount: levelOfUndo)
        self.durationStack = UndoStack<String>(firstItem: timeEntry.duration.safeUnwrapped,
											   maxCount: levelOfUndo)
        self.startTimeStack = UndoStack<String>(firstItem: timeEntry.startTimeString.safeUnwrapped,
												maxCount: levelOfUndo)
        self.endTimeStack = UndoStack<String>(firstItem: timeEntry.endTimeString.safeUnwrapped,
											  maxCount: levelOfUndo)
    }

    func update(with item: TimeEntryViewItem) {
		descriptionEntryStack.push(item.descriptionEntry().safeUnwrapped)
		projectNameStack.push(item.projectAndTaskLabel.safeUnwrapped)
		durationStack.push(item.duration.safeUnwrapped)
		startTimeStack.push(item.startTimeString.safeUnwrapped)
		endTimeStack.push(item.endTimeString.safeUnwrapped)
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
