//
//  TimelineActivity.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 6/21/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

final class TimelineActivity {

    // MARK: Public

    func timechunk() -> TimeChunk? {
        return TimeChunk(start: Date().timeIntervalSince1970 - 200,
                         end: Date().timeIntervalSince1970)
    }
}
