//
//  Collection+Safe.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 2/27/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

extension Collection {

    /// Returns the element at the specified index iff it is within bounds, otherwise nil.
    subscript (safe index: Index) -> Element? {
        return indices.contains(index) ? self[index] : nil
    }
}
