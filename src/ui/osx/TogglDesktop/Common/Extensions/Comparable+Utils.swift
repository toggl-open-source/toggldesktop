//
//  Comparable+Utils.swift
//  TogglDesktop
//
//  Created by Andrew Nester on 30.09.2020.
//  Copyright © 2020 Toggl. All rights reserved.
//

import Foundation

extension Comparable {

    func clamp(min: Self, max: Self) -> Self {
        if self > max {
            return max
        } else if self < min {
            return min
        }

        return self
    }
}
