//
//  TagStorage.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 4/5/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

extension Notification.Name {
    static let TagStorageChangedNotification = Notification.Name("TagStorageChangedNotification")
}


@objcMembers final class TagStorage: NSObject {

    static let shared = TagStorage()

    // MARK: Variables

    private(set) var tags: [Tag] = []

    // MARK: Public

    func update(with viewItems: [ViewItem]) {
        self.tags = build(from: viewItems)
        NotificationCenter.default.post(name: .TagStorageChangedNotification,
                                        object: tags)
    }

    func filter(with text: String) -> [Tag] {
        let filters = tags.filter { $0.name.lowercased().contains(text.lowercased()) }

        if filters.isEmpty {
            return [Tag.noMatching]
        }

        return filters
    }
}

extension TagStorage {

    fileprivate func build(from viewItems: [ViewItem]) -> [Tag] {
        return viewItems.map { Tag(viewItem: $0) }
    }
}

