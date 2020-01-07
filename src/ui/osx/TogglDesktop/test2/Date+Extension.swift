//
//  Date+Extension.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 7/2/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

extension Date {

    static func startOfDay(from timestamp: TimeInterval) -> TimeInterval {
        let date = Date(timeIntervalSince1970: timestamp)
        var calendar = Calendar.current
        calendar.timeZone = TimeZone.current
        return calendar.startOfDay(for: date).timeIntervalSince1970
    }
}
