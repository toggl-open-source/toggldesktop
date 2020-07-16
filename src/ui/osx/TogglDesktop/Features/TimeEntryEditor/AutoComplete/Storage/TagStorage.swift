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
    private var newTags: [Tag] = []

    // MARK: Public

    func update(with viewItems: [ViewItem]) {
        build(from: viewItems) {[weak self] tags in
            guard let strongSelf = self else { return }
            strongSelf.tags = tags
            NotificationCenter.default.post(name: .TagStorageChangedNotification, object: tags)
        }
    }

    func filter(with text: String) -> [Tag] {
        let filters = tags.filter { $0.name.lowercased().contains(text.lowercased()) }

        if filters.isEmpty {
            return [Tag.noMatching]
        }

        return filters
    }

    func addNewTag(_ tag: Tag) {
        newTags.append(tag)
    }
}

extension TagStorage {

    fileprivate func build(from viewItems: [ViewItem], complete: @escaping ([Tag]) -> Void) {
        DispatchQueue.global(qos: .background).async {[weak self] in
            guard let strongSelf = self else { return }
            var tags = viewItems.map { Tag(viewItem: $0) }

            // We have to add new tags manually
            // It's a bug in Library
            // There is no new tags, even it synced properly, until we open the TimeEntryEditor again
            strongSelf.newTags.forEach { tag in
                if !tags.contains(where: { $0.name == tag.name }) {
                    tags.append(tag)
                }
            }
            DispatchQueue.main.async {
                complete(tags)
            }
        }
    }
}

