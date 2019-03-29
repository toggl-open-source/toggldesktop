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

    private(set) var clients: [Client] = []

    // MARK: Public

    func update(with viewItems: [ViewItem]) {
        self.clients = buildClients(from: viewItems)
        NotificationCenter.default.post(name: .ClientStorageChangedNotification,
                                        object: clients)
    }

    func filter(with text: String) -> [Client] {
        return clients.filter { $0.name.lowercased() == text.lowercased() }
    }
}

extension ClientStorage {

    fileprivate func buildClients(from viewItems: [ViewItem]) -> [Client] {
        return viewItems.map { Client(viewItem: $0) }
    }
}
