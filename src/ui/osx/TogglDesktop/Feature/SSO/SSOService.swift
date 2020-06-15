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
            static let SsoError = "ssoError"
            static let LoginPath = "sso-login"
            static let MustLinkPath = "sso-must-link"
            static let Finish = "finish"
        }

        struct Values {
            static let NoAccount = "noAccount"
            static let NotInWorkspace = "notInWorkspace"
        }
    }

    static let shared = SSOService()

    enum Route {
        case success
        case successButMustLink
        case emailDoNotExist
        case notInWorkspace
        case none
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
        guard let component = URLComponents(url: url, resolvingAgainstBaseURL: false) else {
            NotificationCenter.default.post(name: NSNotification.Name(rawValue: kDisplayError), object: "Invalid Callback URL \(url.absoluteString)")
            return
        }

        // https://www.notion.so/toggl/SSO-API-e791bd2451f84ccba43f2e1adba8d85f?p=6c0a5301131747c18be276bbcc211d57
        // It's where we determine what the next steps are:
        // 1 - Successful login (user authenticated on IdP and has linked or newly created account in Toggl)
        // 2 - First successful login with existing Toggl account with the same email - needs confirmation before link account
        // 3 - Toggl account with this email doesn't exist
        // 4 - User is not part of the SSO workspace
        //
        // The Endpoint from Backend is not finished yet
        // Prepare and update later
        let route = getRoute(with: component)
        handle(route)
    }
}

// MARK: Private

extension SSOService {

    private func getRoute(with component: URLComponents) -> Route {
        // Get the first path
        guard let firstPath = component.path.components(separatedBy: "/").first else {
            NotificationCenter.default.post(name: NSNotification.Name(rawValue: kDisplayError), object: "Missing Path")
            return .none
        }

        switch firstPath {
        case Constants.Keys.LoginPath:

            // Success path
            if let finish = component.queryItems?.first(where: { $0.name == Constants.Keys.Finish }), finish.value == "true" {
                return Route.success
            }

            // Check error
            guard let errorCode = component.queryItems?.first(where: { $0.name == Constants.Keys.SsoError }),
                let value = errorCode.value else {
                NotificationCenter.default.post(name: NSNotification.Name(rawValue: kDisplayError), object: "Missing SSO error code")
                return .none
            }

            switch value {
            case Constants.Values.NoAccount:
                return Route.emailDoNotExist
            case Constants.Values.NotInWorkspace:
                return Route.notInWorkspace
            default:
                return .none
            }

        case Constants.Keys.MustLinkPath:
            // Extract the confirmation_code from the callback
            guard let code = component.queryItems?.first(where: { $0.name == Constants.Keys.ConfirmationCode }) else {
                NotificationCenter.default.post(name: NSNotification.Name(rawValue: kDisplayError), object: "Missing confirmation code")
                return .none
            }
            print(code)
        default:
            NotificationCenter.default.post(name: NSNotification.Name(rawValue: kDisplayError), object: "Unsupported path \(firstPath)")
        }
        return .none
    }

    private func handle(_ route: Route) {
        switch route {
        case .emailDoNotExist:
            break
        case .notInWorkspace:
            break
        case .success:
            break
        case .successButMustLink:
            break
        case .none:
            break
        }
    }
}
