//
//  UndoQueue.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 12/11/18.
//  Copyright © 2018 Alari. All rights reserved.
//

import Foundation

final class UndoQueue<T: Equatable> {

    // MARK: - Variable
    private var storage: [T]
    private let maxCount: Int

    // MARK: - Init
    init(storage: [T], maxCount: Int = 2) {
        self.storage = storage
        self.maxCount = maxCount
    }

    // MARK: - Public
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

    func dequeue() -> T? {
        let last =  storage.last
        storage.removeLast()
        return last
    }

    func undoItem() -> T? {
        guard storage.count > 1 else { return nil }
        return storage[1]
    }
}
