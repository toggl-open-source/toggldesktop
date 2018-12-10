//
//  TableViewDiffer.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 12/10/18.
//  Copyright © 2018 Alari. All rights reserved.
//

import Foundation
import DeepDiff
import Cocoa

/// TableViewDiffer
/// An helper struct to execute diff algorithm
// Then reloading with NSTableView approprately
struct TableViewDiffer {

    /// Diffing and reloading func
    /// We don't expose this func (by using @objc)
    /// Due to the limit of Objc (<T> is unable to expose)
    ///
    /// - Parameters:
    ///   - old: Old value
    ///   - new: New Value
    ///   - tableView: The primary tableview
    func diff<T: Hashable>(_ old: [T], new: [T], with tableView: NSTableView) {

        // Use Deep-Diff to diff the change
        let changed = DeepDiff.diff(old: old, new: new, algorithm: WagnerFischer())

        // Early exit if there is no changes
        guard !changed.isEmpty else { return }

        // Reload with changes
        tableView.reload(changes: changed)
    }
}
