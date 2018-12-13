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

protocol UndoItemType {

    func update(with item: Any)
}

final class UndoQueue<T: Equatable & UndoItemType> {

    // MARK: - Variable
    private var storage: [T]
    private let maxCount = 2

    // MARK: - Init
    init(storage: [T]) {
        self.storage = storage
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

        // Update update value with new one
        var temps = storage
        if let first = storage.first {
            if storage.count == maxCount {
                first.update(with: object)
            } else {
                temps.insert(object, at: 0)
            }
        } else {
            temps.append(object)
        }

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
