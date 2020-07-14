//
//  SSOService.swift
//  TogglTrack
//
//  Created by Nghia Tran on 6/10/20.
//  Copyright © 2020 Alari. All rights reserved.
//

import Foundation

final class SSOService {

    private struct Constants {
        struct CallbackHost {
            static let LoginPath = "sso-login"
            static let MustLinkPath = "sso-must-link"
        }
        struct CallbackQuery {
            static let ConfirmationCode = "confirmation_code"
            static let Email = "email"
            static let SsoError = "ssoError"
            static let APIToken = "apiToken"
        }

        struct ErrorKeys {
            static let NoAccount = "noAccount"
            static let NotInWorkspace = "notInWorkspace"
        }
    }

    enum Route {
        case success(String)
        case needLinkSSO(String, String)
        case error(String)
    }

    static let shared = SSOService()

    private init() {}

    // MARK: Public

    func authorize(with urlStr: String) {
        guard let url = URL(string: urlStr) else {
            present(error: "Invalid URL")
            return
        }
        // open with external browser
        NSWorkspace.shared.open(url)
    }

    func handleCallback(with url: URL) {
        // Determine if it's the valid URL callback
        guard let components = URLComponents(url: url, resolvingAgainstBaseURL: false) else {
            present(error: "Invalid Callback URL \(url.absoluteString)")
            return
        }

        // https://www.notion.so/toggl/SSO-API-e791bd2451f84ccba43f2e1adba8d85f?p=6c0a5301131747c18be276bbcc211d57
        // Handle next steps from the URL callback
        let route = getRoute(with: components)
        handle(route)
    }
}

// MARK: Private

extension SSOService {

    private func getRoute(with components: URLComponents) -> Route {
        // Get the first path
        guard let host = components.host else {
            return .error("Missing Path")
        }

        switch host {
        case Constants.CallbackHost.LoginPath: // sso-login

            // Success path
            if let apiTokenQuery = components.queryItems?.first(where: { $0.name == Constants.CallbackQuery.APIToken }),
                let apiToken = apiTokenQuery.value, !apiToken.isEmpty {
                return .success(apiToken)
            }

            // Check error
            guard let errorCode = components.queryItems?.first(where: { $0.name == Constants.CallbackQuery.SsoError }),
                let value = errorCode.value else {
                    return .error("Missing SSO error code")
            }

            switch value {
            case Constants.ErrorKeys.NoAccount:
                return .error("SSO Email doesn't exist")
            case Constants.ErrorKeys.NotInWorkspace:
                return .error("SSO Email doesn't belong to any workspace")
            default:
                return .error(
                    NSLocalizedString(
                        "Single Sign On is not configured for your email address. Please try a different login method or contact your administrator.",
                        comment: "SSO not configured error message"
                ))
            }
        case Constants.CallbackHost.MustLinkPath: // sso-must-link
            // Extract values from the query
            guard let codeQuery = components.queryItems?.first(where: { $0.name == Constants.CallbackQuery.ConfirmationCode }),
                let code = codeQuery.value  else {
                return .error("Missing Confirmation Code")
            }
            guard let emailQuery = components.queryItems?.first(where: { $0.name == Constants.CallbackQuery.Email }),
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
