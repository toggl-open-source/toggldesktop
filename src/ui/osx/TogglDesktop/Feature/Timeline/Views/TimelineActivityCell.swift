//
//  TimelineActivityCell.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 6/21/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

final class TimelineActivityCell: TimelineBaseCell {

    // MARK: Variables

    private(set) var activity: TimelineActivity?

    // MARK: Public

    func config(for activity: TimelineActivity) {
        self.activity = activity
        renderColor(with: activity.color, isSmallEntry: activity.isSmall)
        initAllTracking()
    }
}
