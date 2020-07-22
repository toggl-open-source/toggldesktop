//
//  WorkspaceStorage.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 4/1/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

extension Notification.Name {
    static let WorkspaceStorageChangedNotification = Notification.Name("WorkspaceStorageChangedNotification")
}


@objcMembers final class WorkspaceStorage: NSObject {

    static let shared = WorkspaceStorage()

    // MARK: Variables

    private(set) var workspaces: [Workspace] = []

    // MARK: Public

    func update(with viewItems: [ViewItem]) {
        self.workspaces = builWorkspace(from: viewItems)
        NotificationCenter.default.post(name: .WorkspaceStorageChangedNotification,
                                        object: workspaces)
    }

    func filter(with text: String) -> [Workspace] {
        let filters = workspaces.filter { $0.name.fuzzySearch(with: text) }

        return filters
    }
}

extension WorkspaceStorage {

    fileprivate func builWorkspace(from viewItems: [ViewItem]) -> [Workspace] {
        return viewItems.map { Workspace(viewItem: $0) }
    }
}
