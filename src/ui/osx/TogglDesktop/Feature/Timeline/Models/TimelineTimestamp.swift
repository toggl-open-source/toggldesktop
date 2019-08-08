//
//  TimelineTimestamp.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 6/21/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

final class TimelineTimestamp {

    let label: String

    // MARK: Init
    
    init(_ time: TimeInterval) {
        let date = Date(timeIntervalSince1970: time)
        label = TimelineDateFormatter.shared.convertToHours(date)
    }
}
