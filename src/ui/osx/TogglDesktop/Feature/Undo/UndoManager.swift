//
//  UndoManager.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 12/11/18.
//  Copyright © 2018 Alari. All rights reserved.
//

import Foundation

@objc final class UndoManager: NSObject {

    private struct Constants {
        static let QueueName = "com.toggl.toggldesktop.TogglDesktop.undo"
    }

    // MARK: - Variable
    @objc static let shared = UndoManager()
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
extension UndoManager {

    @objc func store(with object: TimeEntryViewItem) {
        let payload = TimeEntryUndoPayload(descriptionEntry: object.descriptionEntry(),
                                           project: object.projectAndTaskLabel ?? "")
        if let undo = get(for: object.guid, type: UndoQueue<TimeEntryUndoPayload>.self) {
            undo.enqueue(payload)
        } else {
            let undo = UndoQueue<TimeEntryUndoPayload>(storage: [payload])
            set(undo, for: object.guid)
        }
    }

    @objc func getUndoPayload(for guid: String) -> ObjcTimeEntry? {
        guard let undoQueue = get(for: guid, type: UndoQueue<TimeEntryUndoPayload>.self) else {
            return nil
        }
        return undoQueue.undoItem()?.toObjcTimeEntry()
    }
}
