//
//  Array+View.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 9/3/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

extension Array where Element == NSView {

    func viewWithMaxWidth() -> NSView? {
        let max = self.max { (lhs, rhs) -> Bool in
            return lhs.frame.size.width >= rhs.frame.size.width
        }
        return max
    }
}
