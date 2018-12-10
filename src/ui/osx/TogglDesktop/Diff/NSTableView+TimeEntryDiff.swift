//
//  NSTableView+TimeEntryDiff.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 12/10/18.
//  Copyright © 2018 Alari. All rights reserved.
//

import Cocoa

// MARK: - NSTableView + Diff
extension NSTableView {
    
    /// Compute the diff and reload automatically
    ///
    /// - Parameters:
    ///   - old: Old data source
    ///   - new: New data source
    @objc func diffReload(with old: [TimeEntryViewItem], new: [TimeEntryViewItem]) {
        TableViewDiffer().diff(old, new: new, with: self)
    }
}
