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
		super.init()
    }

    func update(with item: TimeEntryViewItem) {
		descriptionEntryStack.push(item.descriptionEntry().safeUnwrapped)
		projectNameStack.push(item.projectAndTaskLabel.safeUnwrapped)
		durationStack.push(item.duration.safeUnwrapped)
		startTimeStack.push(item.startTimeString.safeUnwrapped)
		endTimeStack.push(item.endTimeString.safeUnwrapped)
    }

	override var debugDescription: String {
		return "\n💥 Description \(descriptionEntryStack)\nProject \(projectNameStack)\nDuration \(durationStack)\nStartTime \(startTimeStack)\nEndTime \(endTimeStack)"
	}
}

// MARK: - Objc interfaces
extension TimeEntrySnapshot {

	@objc var descriptionUndoValue: String? {
		return descriptionEntryStack.undoItem()
	}

	@objc var projectNameUndoValue: String? {
		return projectNameStack.undoItem()
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
