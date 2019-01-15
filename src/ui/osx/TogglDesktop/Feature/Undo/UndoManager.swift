//
//  UndoManager.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 12/11/18.
//  Copyright © 2018 Alari. All rights reserved.
//

import Foundation

/// Custome Undomanager
/// It store any UndoPayload with unique key
@objc final class UndoManager: NSObject {

    private struct Constants {
        static let QueueName = "com.toggl.toggldesktop.TogglDesktop.undo"
    }

    // MARK: - Variable
    @objc static let shared = UndoManager()
    private var storage: [String: Any] = [:]
    private let levelOfUndo: Int
    private lazy var queue: DispatchQueue = {
        let qos: DispatchQoS
        if #available(OSX 10.10, *) {
            qos = .background
        } else {
            qos = .unspecified
        }
        return DispatchQueue(label: Constants.QueueName, qos: qos, attributes: .concurrent)
    }()

    // MARK: - Init
    init(levelOfUndo: Int = 2) {
        self.levelOfUndo = levelOfUndo
    }

    // MARK: - Public

    /// Store payload with unique key
    /// This method is safe-thread by adopting barrier concurrent queue
    ///
    /// - Parameters:
    ///   - object: Store object
    ///   - key: Unique key
    func set<T>(_ object: T, for key: String) {
        queue.async(flags: .barrier) {[unowned self] in
            self.storage[key] = object
        }
    }

    /// Get the undo payload for specific key
    /// This method is safe-thread by adopting barrier concurrent queue
    ///
    /// - Parameters:
    ///   - key: Unique key
    ///   - type: Type of object
    /// - Returns: The optional value of object
    func get<T>(for key: String, type: T.Type) -> T? {
        return queue.sync {
            return storage[key] as? T
        }
    }
}

// MARK: - Extension for TimeEntry

/// Extension for Objc-codebase
/// Because we're unable of expose Generic <T>
extension UndoManager {

    /// Store TimeEntryViewItem
    /// The payload will be enqueue to UndoQueue
    ///
    /// - Parameter object: Time Entry obj
    @objc func store(with item: TimeEntryViewItem) {
		guard let guid = item.guid else { return }

		// Update existing snapshot or create new
		if let snapshot = get(for: guid, type: TimeEntrySnapshot.self) {
			snapshot.update(with: item)
		} else {
			let snapshot = TimeEntrySnapshot(timeEntry: item, levelOfUndo: levelOfUndo)
			set(snapshot, for: guid)
		}
    }

    /// Undo Payload for TimeEntryViewItem
    ///
    /// - Parameter object: TimeEntryViewItem object
    /// - Returns: ObjcTimeEntryPayload
    @objc func getSnapshot(for item: TimeEntryViewItem) -> TimeEntrySnapshot? {
		guard let guid = item.guid else { return nil }
		return get(for: guid, type: TimeEntrySnapshot.self)
    }
}
