//
//  Date+Utils.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 4/19/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

extension Calendar {

    static let gregorian = Calendar(identifier: .gregorian)
    static let utcCalendar: Calendar = {
        var calender = Calendar.current
        calender.timeZone = TimeZone(identifier: "UTC")!
        return calender
    }()
}

extension Date {

    func dayOfWeekString() -> String? {
        guard let weekday = Calendar.current.dateComponents([.weekday], from: self).weekday else {
            return nil
        }
        return Calendar.current.weekdaySymbols[safe: weekday - 1]
    }

    func previousDate() -> Date? {
        return Calendar.current.date(byAdding: .day, value: -1, to: self)
    }

    func nextDate() -> Date? {
        return Calendar.current.date(byAdding: .day, value: 1, to: self)
    }

    func firstDayOfWeek() -> Date? {
        return Calendar.current.date(from: Calendar.gregorian.dateComponents([.yearForWeekOfYear, .weekOfYear], from: self))
    }

    func toLocalTime() -> Date {
        let timezone = TimeZone.current
        let seconds = TimeInterval(timezone.secondsFromGMT(for: self))
        return Date(timeInterval: seconds, since: self)
    }
}
