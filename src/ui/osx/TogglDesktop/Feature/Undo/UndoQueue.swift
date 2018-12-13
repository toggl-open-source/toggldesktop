//
//  UndoQueue.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 12/11/18.
//  Copyright © 2018 Alari. All rights reserved.
//

import Foundation

/// UndoQueue represent the History of value
/// It's kind of Circular Queues
/// The enquene new value, it will drop the last value if it excesses the maxCount
final class UndoQueue<T: Equatable> {

    // MARK: - Variable
    private var storage: [T]
    private let maxCount: Int

    // MARK: - Init
    init(storage: [T], maxCount: Int) {
        self.storage = storage
        self.maxCount = maxCount
    }

    // MARK: - Public
    /// Enqueue object
    /// Store value and drop the last value if need
    ///
    /// - Parameter object: Object
    func enqueue(_ object: T) {

        // We don't store if next object is same the first one
        guard object != storage.first else {
            return
        }

        // Insert at top
        var temps = storage
        temps.insert(object, at: 0)

        // Drop last it's excess the maxCount
        if temps.count > maxCount {
            temps = Array(temps.dropLast())
        }
        storage = temps
    }

    /// Dequeue last object
    ///
    /// - Returns: The object has been dropped
    func dequeue() -> T? {
        let last =  storage.last
        storage.removeLast()
        return last
    }

    /// Get second-last object
    /// It's undo value
    ///
    /// - Returns: The object
    func undoItem() -> T? {
        guard storage.count > 1 else { return nil }
        return storage[1]
    }
}
