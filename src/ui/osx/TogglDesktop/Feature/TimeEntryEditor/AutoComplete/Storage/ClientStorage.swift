//
//  ClientStorage.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 3/29/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

extension Notification.Name {
    static let ClientStorageChangedNotification = Notification.Name("ClientStorageChangedNotification")
}


@objcMembers final class ClientStorage: NSObject {

    static let shared = ClientStorage()

    // MARK: Variables

    private var clients: [Client] = []

    // MARK: Public

    func update(with viewItems: [ViewItem]) {
        self.clients = buildClients(from: viewItems)
        NotificationCenter.default.post(name: .ClientStorageChangedNotification,
                                        object: clients)
    }

    func getClients(at workspace: Workspace?) -> [Client] {
        guard let workspace = workspace else {
             return self.clients
        }
        return clients.filter { $0.workspaceName == workspace.name }
    }

    func filter(with text: String, at workspace: Workspace?) -> [Client] {
        let selectedClients = getClients(at: workspace)
        let filters = selectedClients.filter { $0.name.fuzzySearch(with: text) }
        if filters.isEmpty {
            return [Client.noMatching]
        }

        return filters
    }

    func client(with name: String) -> Client? {
        return clients.first { $0.name == name }
    }
}

extension ClientStorage {

    fileprivate func buildClients(from viewItems: [ViewItem]) -> [Client] {
        return viewItems.map { Client(viewItem: $0) }
    }
}
