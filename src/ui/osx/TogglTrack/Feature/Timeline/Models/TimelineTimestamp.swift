//
//  TimelineTimestamp.swift
//  TogglTrack
//
//  Created by Nghia Tran on 6/21/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

/// Represent the Time Label on the Timeline View
/// Ex: 7:00, 7:15, 7:30
struct TimelineTimestamp {

    let label: String

    // MARK: Init
    
    init(_ time: TimeInterval) {
        let date = Date(timeIntervalSince1970: time)
        label = TimelineDateFormatter.shared.convertToHours(date)
    }
}
