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
        struct Keys {
            static let ConfirmationCode = "confirmation_code"
            static let Email = "email"
            static let SsoError = "ssoError"
            static let LoginPath = "sso-login"
            static let MustLinkPath = "sso-must-link"
            static let APIToken = "apiToken"
        }

        struct Values {
            static let NoAccount = "noAccount"
            static let NotInWorkspace = "notInWorkspace"
        }
    }

    static let shared = SSOService()

    enum Route {
        case success(String)
        case needLinkSSO(String, String)
        case error(String)
    }

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
        // Determine if it's the valid URL callback
        guard let component = URLComponents(url: url, resolvingAgainstBaseURL: false) else {
            NotificationCenter.default.post(name: NSNotification.Name(rawValue: kDisplayError), object: "Invalid Callback URL \(url.absoluteString)")
            return
        }

        // https://www.notion.so/toggl/SSO-API-e791bd2451f84ccba43f2e1adba8d85f?p=6c0a5301131747c18be276bbcc211d57
        // Handle next steps from the URL callback
        let route = getRoute(with: component)
        handle(route)
    }
}

// MARK: Private

extension SSOService {

    private func getRoute(with component: URLComponents) -> Route {
        // Get the first path
        guard let host = component.host else {
            return .error("Missing Path")
        }

        switch host {
        case Constants.Keys.LoginPath: // sso-login

            // Success path
            if let apiTokenQuery = component.queryItems?.first(where: { $0.name == Constants.Keys.APIToken }),
                let apiToken = apiTokenQuery.value, !apiToken.isEmpty {
                return .success(apiToken)
            }

            // Check error
            guard let errorCode = component.queryItems?.first(where: { $0.name == Constants.Keys.SsoError }),
                let value = errorCode.value else {
                    return .error("Missing SSO error code")
            }

            switch value {
            case Constants.Values.NoAccount:
                return .error("SSO Email doesn't exist")
            case Constants.Values.NotInWorkspace:
                return .error("SSO Email isn't belong to any workspace")
            default:
                return .error("SSO Error")
            }
        case Constants.Keys.MustLinkPath: // sso-must-link
            // Extract values from the query
            guard let codeQuery = component.queryItems?.first(where: { $0.name == Constants.Keys.ConfirmationCode }),
                let code = codeQuery.value  else {
                return .error("Missing Confirmation Code")
            }
            guard let emailQuery = component.queryItems?.first(where: { $0.name == Constants.Keys.Email }),
                let email = emailQuery.value  else {
                return .error("Missing SSO Email")
            }
            return .needLinkSSO(email, code)
        default:
            break
        }
        return .error("Unsupported path \(host)")
    }

    private func handle(_ route: Route) {
        switch route {
        case .error(let message):
            present(error: message)
        case .success(let apiToken):
            DesktopLibraryBridge.shared().loginSSO(withAPIToken: apiToken)
        case .needLinkSSO(let email, let code):
            NotificationCenter.default.post(name: Notification.Name(rawValue: kLinkSSOEmail), object: SSOPayload(email: email, confirmationCode: code))
        }
    }

    private func present(error: String) {
        NotificationCenter.default.post(name: Notification.Name(rawValue: kDisplayError), object: error)
    }
}
