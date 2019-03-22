//
//  SystemMessage.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 3/1/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

protocol SystemMessagePresentable {

    func dismiss(_ payload: SystemMessage.Payload)
    func present(_ payload: SystemMessage.Payload)
}

@objc final class SystemMessage: NSObject {

    enum Mode {
        case offline
        case syncing
        case error
        case information
    }

    enum Content {
        case error(String, String?) // Title + subtitle
        case informative(String)
    }

    struct Payload {

        let mode: Mode
        let content: Content
    }

    // MARK: Variable

    @objc static let shared = SystemMessage()
    private var presenter: SystemMessagePresentable?

    // Public

    func present(_ payload: SystemMessage.Payload) {
        presenter?.present(payload)
    }

    func dismiss(_ payload: SystemMessage.Payload) {
        presenter?.dismiss(payload)
    }

    func register(for presenter: SystemMessagePresentable) {
        self.presenter = presenter
    }
}

// MARK: Objc extension
// We don't need to expose Payload and Mode struct/enum to objc

extension SystemMessage {

    @objc func presentError(_ title: String, subTitle: String?) {
        let payload = Payload(mode: .error,
                              content: .error(title, subTitle))
        present(payload)
    }

    @objc func presentOffline(_ title: String, subTitle: String?) {
        let payload = Payload(mode: .offline,
                              content: .error(title, subTitle))
        present(payload)
    }

    @objc func presentSyncing() {
        let payload = Payload(mode: .syncing,
                              content: .informative("Syncing..."))
        present(payload)
    }

    @objc func dismissSyncing() {
        let payload = Payload(mode: .syncing,
                              content: .informative("Syncing..."))
        dismiss(payload)
    }
}
