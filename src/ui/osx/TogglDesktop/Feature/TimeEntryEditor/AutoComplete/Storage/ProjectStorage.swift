//
//  ProjectStorage.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 3/25/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

extension Notification.Name {
    static let ProjectStorageChangedNotification = Notification.Name("ProjectStorageChangedNotification")
}

@objcMembers final class ProjectStorage: NSObject {

    static let shared = ProjectStorage()

    // MARK: Variables

    private(set) var items: [Any] = []

    // MARK: Public

    func update(with autoCompleteItems: [AutocompleteItem]) {
        guard let firstItem = autoCompleteItems.first else {
            items = []
            NotificationCenter.default.post(name: .ProjectStorageChangedNotification, object: items)
            return
        }

        // Process
        var newItems: [Any] = []
        var currentClient = firstItem.clientLabel

        // Append the first client
        newItems.append(ProjectHeaderItem(item: firstItem))

        // Append new item or create new client
        for item in autoCompleteItems {
            if item.clientLabel != currentClient {
                newItems.append(ProjectHeaderItem(item: item))
                currentClient = item.clientLabel
            }
            newItems.append(ProjectRowItem(item: item))
        }

        // Notify
        items = newItems
        NotificationCenter.default.post(name: .ProjectStorageChangedNotification, object: items)
    }
}
