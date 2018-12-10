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
        let changed = DeepDiff.diff(old: old, new: new)

        // Early exit if there is no changes
        guard !changed.isEmpty else { return }

        // Reload with changes
        tableView.reload(changes: changed)
    }
}

/// EntryTableViewDiffer
/// An helper class to expose generic func from TableViewDiffer to Objc codebase
@objc final class EntryTableViewDiffer: NSObject {

    /// Calculate the diff and reload
    ///
    /// - Parameters:
    ///   - old: Old array (conform Hashable)
    ///   - new: New array (conform Hashable)
    ///   - tableView: A primary NSTableView
    @objc func reload(_ old: [TimeEntryViewItem], new: [TimeEntryViewItem], with tableView: NSTableView) {
        TableViewDiffer().diff(old, new: new, with: tableView)
    }
}
