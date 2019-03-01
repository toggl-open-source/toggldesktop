//
//  SystemMessage.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 3/1/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

protocol SystemMessagePresentable {

    func present(_ payload: SystemMessage.Payload)
}

@objc final class SystemMessage: NSObject {

    enum Mode {
        case offline
        case syncing
        case error
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

    func register(for presenter: SystemMessagePresentable) {
        self.presenter = presenter
    }
}
