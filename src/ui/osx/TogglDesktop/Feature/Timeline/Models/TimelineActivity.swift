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
    let durationStr: String
    let isHeader: Bool
    let subEvents: [TimelineEvent]

    // MARK: Variables

    init(title: String, fileName: String, duration: TimeInterval, durationStr: String, isHeader: Bool, subEvents: [TimelineEvent]) {
        self.title = title
        self.fileName = fileName
        self.duration = duration
        self.durationStr = durationStr
        self.isHeader = isHeader
        self.subEvents = subEvents
    }
}

final class TimelineActivity {

    private static let LightestBlueColor = NSColor(red:0.702, green:0.898, blue:0.988, alpha:1.0)
    private static let DarkestBlueColor = NSColor(red:0.020, green:0.667, blue:0.961, alpha:1.0)

    // MARK: Variables

    let started: TimeInterval
    let ended: TimeInterval
    let duration: TimeInterval
    let startTimeStr: String
    let endTimeStr: String
    let events: [TimelineEvent]
    var isSmall = false
    lazy var color: NSColor = {
        let ratio = CGFloat(duration / 900.0)
        return TimelineActivity.getColor(from: TimelineActivity.LightestBlueColor, darkest: TimelineActivity.DarkestBlueColor, ratio: ratio)
    }()

    // MARK: Init

    init(started: TimeInterval, duration: TimeInterval, startTimeStr: String, endTimeStr: String, events: [TimelineEvent]) {
        self.started = started
        self.duration = duration
        self.startTimeStr = startTimeStr
        self.endTimeStr = endTimeStr
        self.events = events
        self.ended = started + duration
    }

    // MARK: Public

    func timechunk() -> TimeChunk? {
        return TimeChunk(start: started, end: ended)
    }

    private static func getColor(from lightest: NSColor, darkest: NSColor, ratio: CGFloat) -> NSColor {
        let red = darkest.redComponent + (lightest.redComponent - darkest.redComponent) * ratio
        let blue = darkest.blueComponent + (lightest.blueComponent - darkest.blueComponent) * ratio
        let green = darkest.greenComponent + (lightest.greenComponent - darkest.greenComponent) * ratio
        return NSColor(red: red, green: green, blue: blue, alpha: 1.0)
    }
}
