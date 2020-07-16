//
//  TimelineDateFormatter.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 6/24/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

final class TimelineDateFormatter {

    static let shared = TimelineDateFormatter()

    // MARK: Variables

    private let formatter: DateFormatter

    // MARK: Init

    init() {
        formatter = DateFormatter()
        formatter.locale = Locale.current
        formatter.timeZone = TimeZone.current
    }

    // MARK: Public

    /// Convert the date into human-readable text
    /// - Parameter date: Date
    /// - Returns: Date text with hh:mm a format
    func convertToHours(_ date: Date) -> String {
        //TODO: Use use's preference to convert time/date properly
        // https://github.com/toggl-open-source/toggldesktop/issues/4110
        formatter.dateFormat = "hh:mm a"
        return formatter.string(from: date)
    }
}
