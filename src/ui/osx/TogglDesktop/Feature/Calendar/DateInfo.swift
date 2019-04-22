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
    let date: Date
    var isFirstDayOfMonth: Bool {
        return day == 1
    }
    
    init(date: Date) {
        let calendar = Calendar.current
        self.date = date
        day = calendar.component(.day, from: date)
        month = calendar.component(.month, from: date)
        year = calendar.component(.year, from: date)
        weekOfYear = calendar.component(Calendar.Component.weekOfYear, from: date)
        monthTitle = calendar.shortMonthSymbols[month]
    }
}
