//
//  TimeEntrySnapshot.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 12/11/18.
//  Copyright © 2018 Alari. All rights reserved.
//

import Foundation

/// The payload contain the unfo value
@objc final class TimeEntrySnapshot: NSObject {

    // MARK: - Variable
    private let descriptionEntryStack: UndoStack<String>
    private let projectStack: UndoStack<ProjectSnapshot>
    private let durationStack: UndoStack<String>
    private let startTimeStack: UndoStack<String>
    private let endTimeStack: UndoStack<String>

    // MARK: - Init
    init(timeEntry: TimeEntryViewItem, levelOfUndo: Int) {
        self.descriptionEntryStack = UndoStack<String>(firstItem: timeEntry.descriptionEntry().safeUnwrapped,
													   maxCount: levelOfUndo)
        self.projectStack = UndoStack<ProjectSnapshot>(firstItem: ProjectSnapshot(timeEntry: timeEntry),
													   maxCount: levelOfUndo)
        self.durationStack = UndoStack<String>(firstItem: timeEntry.duration.safeUnwrapped,
											   maxCount: levelOfUndo)
        self.startTimeStack = UndoStack<String>(firstItem: timeEntry.startTimeString.safeUnwrapped,
												maxCount: levelOfUndo)
        self.endTimeStack = UndoStack<String>(firstItem: timeEntry.endTimeString.safeUnwrapped,
											  maxCount: levelOfUndo)
		super.init()
    }

    func update(with item: TimeEntryViewItem) {
		descriptionEntryStack.push(item.descriptionEntry().safeUnwrapped)
		projectStack.push(ProjectSnapshot(timeEntry: item))
		durationStack.push(item.duration.safeUnwrapped)
		startTimeStack.push(item.startTimeString.safeUnwrapped)
		endTimeStack.push(item.endTimeString.safeUnwrapped)
    }

	override var debugDescription: String {
		return "\n💥 Description \(descriptionEntryStack)\nProject \(projectStack)\nDuration \(durationStack)\nStartTime \(startTimeStack)\nEndTime \(endTimeStack)"
	}
}

// MARK: - Objc interfaces
extension TimeEntrySnapshot {

	@objc var descriptionUndoValue: String? {
		return descriptionEntryStack.undoItem()
	}

	@objc var projectLableUndoValue: ProjectSnapshot? {
		return projectStack.undoItem()
	}

	@objc var durationUndoValue: String? {
		return durationStack.undoItem()
	}

	@objc var startTimeUndoValue: String? {
		return startTimeStack.undoItem()
	}

	@objc var endTimeUndoValue: String? {
		return endTimeStack.undoItem()
	}
}

@objcMembers class ProjectSnapshot: NSObject {

	let projectAndTaskLabel: String
	let taskID: UInt64
	let projectID: UInt64
    let projectLabel: String
    let projectColor: String

	init(timeEntry: TimeEntryViewItem) {
		self.projectAndTaskLabel = timeEntry.projectAndTaskLabel.safeUnwrapped
		self.taskID = timeEntry.taskID
		self.projectID = timeEntry.projectID
        self.projectLabel = timeEntry.projectLabel
        self.projectColor = timeEntry.projectColor
		super.init()
	}

	override func isEqual(_ object: Any?) -> Bool {
		guard let rhs = object as? ProjectSnapshot else {
			return false
		}
		return self.projectAndTaskLabel == rhs.projectAndTaskLabel &&
			self.taskID == rhs.taskID &&
			self.projectID == rhs.projectID
	}

	static func == (lhs: ProjectSnapshot, rhs: ProjectSnapshot) -> Bool {
		return lhs.projectAndTaskLabel == rhs.projectAndTaskLabel &&
			lhs.taskID == rhs.taskID &&
			lhs.projectID == rhs.projectID
	}

	override var debugDescription: String {
		return "Project Name: \(projectAndTaskLabel)"
	}
}
