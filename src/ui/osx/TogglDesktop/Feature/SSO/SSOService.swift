//
//  SSOService.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 6/10/20.
//  Copyright © 2020 Alari. All rights reserved.
//

import Foundation

final class SSOService {

    private struct Constants {
        static let TokenKey = "token"
    }

    static let shared = SSOService()

    // MARK: Public

    func authorize(with urlStr: String) {
        guard let url = URL(string: urlStr) else {
            NotificationCenter.default.post(name: NSNotification.Name(rawValue: kDisplayError), object: "Invalid URL")
            return
        }
        // open with external browser
        NSWorkspace.shared.open(url)
    }

    func handleCallback(with url: URL) {
        guard let component = URLComponents(url: url, resolvingAgainstBaseURL: false) else {
            NotificationCenter.default.post(name: NSNotification.Name(rawValue: kDisplayError), object: "Invalid Callback URL \(url.absoluteString)")
            return
        }

        // Extract the token from the callback
        guard let token = component.queryItems?.first(where: { $0.name == Constants.TokenKey }) else {
            NotificationCenter.default.post(name: NSNotification.Name(rawValue: kDisplayError), object: "Missing Token KeyValue \(url.absoluteString)")
            return
        }

        print(token)
    }
}
