//
//  DateInfo.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 4/19/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

struct DateInfo {

    let day: Int
    let month: Int
    let monthTitle: String
    let year: Int
    let weekOfYear: Int
    var isFirstDayOfMonth: Bool {
        return day == 1
    }
    
    init(date: Date) {
        let calendar = Calendar.current
        let component = calendar.dateComponents([.day, .month, .year, .weekOfYear], from: date)
        day = component.day ?? 0
        month = component.month ?? 0
        year = component.year ?? 0
        weekOfYear = component.weekOfYear ?? 0
        monthTitle = calendar.shortMonthSymbols[month - 1]
    }

    init(day: Int, month: Int, monthTitle: String, year: Int, weekOfYear: Int) {
        self.day = day
        self.month = month
        self.monthTitle = monthTitle
        self.year = year
        self.weekOfYear = weekOfYear
    }

    func isSameDay(with info: DateInfo) -> Bool {
        return day == info.day && month == info.month && year == info.year
    }
}
