//
//  TimelineDraggingSession.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 2/10/20.
//  Copyright © 2020 Alari. All rights reserved.
//

import Foundation

struct TimelineDraggingSession {

    // Selected indexpath for dragging item
    var indexPath: IndexPath?

    // The distance between the mouse and the Top of the TimeEntry pill
    var mouseDistanceFromTop: Double?

    // Start Time after dragging
    private(set) var finalStartTime: TimeInterval?
    private(set) var item: TimelineTimeEntry?

    // Determine if the user is dragging
    var isDragging: Bool {
        return indexPath != nil
    }

    mutating func reset() {
        indexPath = nil
        mouseDistanceFromTop = nil
        finalStartTime = nil
        item = nil
    }

    mutating func acceptDrop(at time: TimeInterval, for timeEntry: TimelineTimeEntry) {
        finalStartTime = time
        item = timeEntry
    }
}
