//
//  TimelineInitialDateProvider.swift
//  TogglTrack
//
//  Created by Andrew Nester on 01.06.2020.
//  Copyright © 2020 Alari. All rights reserved.
//

import Foundation

/// Calculates what date to show to the user when opening the Timeline screen
/// based on a last user session and current date.
struct TimelineInitialDateProvider {

    private struct Constants {
        static let lastTimelineActiveDateKey = "lastTimelineActiveDate"
    }

    var dateUpdatedHandler: ((Date) -> Void)?

    // TODO: consider using @propertyWrapper's for UserDefaults storage
    private var lastActiveDate: Date {
        get {
            let timeInterval = defaults.double(forKey: Constants.lastTimelineActiveDateKey)
            return Date(timeIntervalSince1970: timeInterval)
        }
        set {
            defaults.set(Date().timeIntervalSince1970, forKey: Constants.lastTimelineActiveDateKey)
        }
    }

    private var defaults: UserDefaults

    init(defaults: UserDefaults = .standard, dateUpdatedHandler: ((Date) -> Void)?) {
        self.defaults = defaults
        self.dateUpdatedHandler = dateUpdatedHandler
    }

    func timelineOpened() {
        if Calendar.current.isDateInToday(lastActiveDate) {
            return
        }
        dateUpdatedHandler?(Date())
    }

    mutating func timelineClosed() {
        lastActiveDate = Date()
    }
}
