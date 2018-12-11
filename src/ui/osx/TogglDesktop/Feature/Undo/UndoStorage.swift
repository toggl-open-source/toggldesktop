//
//  UndoStorage.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 12/11/18.
//  Copyright © 2018 Alari. All rights reserved.
//

import Foundation

@objc final class UndoStorage: NSObject {

    private struct Constants {
        static let QueueName = "com.toggl.toggldesktop.TogglDesktop.undo"
    }

    // MARK: - Variable
    static let shared = UndoStorage()
    private var storage: [String: Any] = [:]
    private lazy var queue: DispatchQueue = {
        let qos: DispatchQoS
        if #available(OSX 10.10, *) {
            qos = .background
        } else {
            qos = .unspecified
        }
        return DispatchQueue(label: Constants.QueueName, qos: qos, attributes: .concurrent)
    }()

    // MARK: - Public
    func set<T>(_ object: T, for key: String) {
        queue.async(flags: .barrier) {[unowned self] in
            self.storage[key] = object
        }
    }

    func get<T>(for key: String, type: T.Type) -> T? {
        return queue.sync {
            return storage[key] as? T
        }
    }
}

// MARK: - Extension for TimeEntry
extension UndoStorage {

    @objc func set(_ object: TimeEntryViewItem) {
        set(object, for: object.guid)
    }

    @objc func get(for key: String) -> TimeEntryViewItem? {
        return get(for: key, type: TimeEntryViewItem.self)
    }
}
