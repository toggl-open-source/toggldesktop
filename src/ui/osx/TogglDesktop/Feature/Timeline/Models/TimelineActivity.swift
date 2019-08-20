//
//  TimelineActivity.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 6/21/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

final class TimelineEvent {

    // MARK: Variables

    let title: String
    let fileName: String
    let duration: TimeInterval
    let isHeader: Bool
    let subEvents: [TimelineEvent]

    // MARK: Variables

    init(title: String, fileName: String, duration: TimeInterval, isHeader: Bool, subEvents: [TimelineEvent]) {
        self.title = title
        self.fileName = fileName
        self.duration = duration
        self.isHeader = isHeader
        self.subEvents = subEvents
    }
}

final class TimelineActivity {

    // MARK: Variables

    let started: TimeInterval
    let ended: TimeInterval
    let duration: TimeInterval
    let startTimeStr: String
    let events: [TimelineEvent]
    var isSmall = false

    // MARK: Init

    init(started: TimeInterval, duration: TimeInterval, startTimeStr: String, events: [TimelineEvent]) {
        self.started = started
        self.duration = duration
        self.startTimeStr = startTimeStr
        self.events = events
        self.ended = started + duration
    }

    // MARK: Public

    func timechunk() -> TimeChunk? {
        return TimeChunk(start: started, end: ended)
    }
}
