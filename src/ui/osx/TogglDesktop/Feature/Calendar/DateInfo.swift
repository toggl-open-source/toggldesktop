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
    let year: Int
    let weekOfYear: Int
    
    init(date: Date) {
        let calendar = Calendar.current
        day = calendar.component(.day, from: date)
        month = calendar.component(.month, from: date)
        year = calendar.component(.year, from: date)
        weekOfYear = calendar.component(Calendar.Component.weekOfYear, from: date)
    }
}
