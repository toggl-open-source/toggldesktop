//
//  SystemMessage.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 3/1/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

protocol SystemMessagePresentable {

    func present(_ message: SystemMessage.Message)
}

final class SystemMessage {

    enum Message {
        case error(String, String) // Title + subtitle
        case informative(String)
    }

    // MARK: Variable

    static let shared = SystemMessage()
    private var presenter: SystemMessagePresentable?

    // Public

    func present(_ message: Message) {
        presenter?.present(message)
    }

    func register(for presenter: SystemMessagePresentable) {
        self.presenter = presenter
    }
}
