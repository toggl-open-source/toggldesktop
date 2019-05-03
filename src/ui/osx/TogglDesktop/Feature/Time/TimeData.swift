//
//  TimeData.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 5/3/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

struct TimeData {

    let date: Date
    let hour: Int
    let minute: Int
    let second: Int

    init(date: Date) {
        let calendar = Calendar.current
        let component = calendar.dateComponents([.second, .minute, .hour], from: date)
        self.date = date
        self.hour = component.hour ?? 0
        self.minute = component.minute ?? 0
        self.second = component.second ?? 0
    }
}
