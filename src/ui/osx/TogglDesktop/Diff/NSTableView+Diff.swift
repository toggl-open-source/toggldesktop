//
//  NSTableView+Diff.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 12/10/18.
//  Copyright © 2018 Alari. All rights reserved.
//

import Cocoa
import DeepDiff

/// Represent the changes
struct ChangeWithIndexPath {

    public let inserts: [IndexSet]
    public let deletes: [IndexSet]
    public let replaces: [IndexSet]
    public let moves: [(from: Int, to: Int)]
}

/// Helper class to convert the DeepDiff.Change to ChangeWithIndexPath
struct IndexPathConverter {

    /// Convert the DeepDiff.Change enum
    /// To helper class ChangeWithIndexPath
    ///
    /// - Parameter changes: The enum changes
    /// - Returns: ChangeWithIndexPath
    func convert<T>(changes: [Change<T>]) -> ChangeWithIndexPath {
        let inserts = changes.compactMap { $0.insert }.map { IndexSet(integer: $0.index) }
        let deletes = changes.compactMap { $0.delete }.map { IndexSet(integer: $0.index) }
        let replaces = changes.compactMap { $0.replace }.map { IndexSet(integer: $0.index) }
        let moves = changes.compactMap { $0.move }.map { ($0.fromIndex, $0.toIndex) }
        return ChangeWithIndexPath(inserts: inserts,
                                   deletes: deletes,
                                   replaces: replaces,
                                   moves: moves)
    }
}

// MARK: - NSTableView
extension NSTableView {

    /// Reload tableview depends on the diff changes
    ///
    /// - Parameters:
    ///   - changes: The array of Change<T>
    ///   - completion: Complete Block
    func reload<T: Hashable>(changes: [Change<T>], completion: (() -> Void)? = nil) {
        let changesWithIndexPath = IndexPathConverter().convert(changes: changes)

        // Insert or Remove or Move
        beginUpdates()
        internalBatchUpdates(changesWithIndexPath: changesWithIndexPath)
        endUpdates()

        // Replace
        if !changesWithIndexPath.replaces.isEmpty {
            beginUpdates()
            let columnIndex = IndexSet.init(integersIn: 0..<numberOfColumns)
            changesWithIndexPath.replaces.forEach {
                reloadData(forRowIndexes: $0, columnIndexes: columnIndex)
            }
            endUpdates()
        }

        completion?()
    }

    /// Execute removeRows/insertRows/moveRow appropriately
    /// It relies on the input changesWithIndexPath
    ///
    /// - Parameter changesWithIndexPath: The IndexSet changes
    private func internalBatchUpdates(changesWithIndexPath: ChangeWithIndexPath) {

        changesWithIndexPath.deletes.reversed().forEach {
            removeRows(at: $0, withAnimation: .effectFade)
        }

        changesWithIndexPath.inserts.forEach {
            insertRows(at: $0, withAnimation: .effectFade)
        }

        changesWithIndexPath.moves.forEach {
            moveRow(at: $0.from, to: $0.to)
        }
    }
}
