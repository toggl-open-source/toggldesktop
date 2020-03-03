//
//  AppleAuthenticationService.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 3/3/20.
//  Copyright © 2020 Alari. All rights reserved.
//

import Foundation
import AuthenticationServices

@objc protocol AppleAuthenticationServiceDelegate: class {

    func appleAuthenticationPresentOnWindow() -> NSWindow
}

@available(OSX 10.15, *)
@objc
final class AppleAuthenticationService: NSObject {

    static let shared = AppleAuthenticationService()

    // MARK: Variables

    @objc weak var delegate: AppleAuthenticationServiceDelegate?

    // MARK: Public

    func requestApple() {
        let appleIDProvider = ASAuthorizationAppleIDProvider()
        let request = appleIDProvider.createRequest()
        request.requestedScopes = [.fullName, .email]

        let authorizationController = ASAuthorizationController(authorizationRequests: [request])
        authorizationController.delegate = self
        authorizationController.presentationContextProvider = self
        authorizationController.performRequests()
    }
}

// MARK: ASAuthorizationControllerDelegate

@available(OSX 10.15, *)
extension AppleAuthenticationService: ASAuthorizationControllerDelegate {

    func authorizationController(controller: ASAuthorizationController, didCompleteWithError error: Error) {

    }

    func authorizationController(controller: ASAuthorizationController, didCompleteWithAuthorization authorization: ASAuthorization) {
        switch authorization.credential {
        case let appleIDCredential as ASAuthorizationAppleIDCredential:

            // Create an account in your system.
            let userIdentifier = appleIDCredential.user
            let fullName = appleIDCredential.fullName
            let email = appleIDCredential.email

        default:
            break
        }
    }
}

// MARK: ASAuthorizationControllerPresentationContextProviding

@available(OSX 10.15, *)
extension AppleAuthenticationService: ASAuthorizationControllerPresentationContextProviding {

    func presentationAnchor(for controller: ASAuthorizationController) -> ASPresentationAnchor {
        return delegate?.appleAuthenticationPresentOnWindow() ?? NSApplication.shared.mainWindow!
    }
}

