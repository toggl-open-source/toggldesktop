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

    @available(OSX 10.15, *)
    func appleAuthenticationDidComplete(with credential: ASAuthorizationAppleIDCredential)
    func appleAuthenticationDidFailed(with error: Error)
    func appleAuthenticationPresentOnWindow() -> NSWindow
}

@available(OSX 10.15, *)
@objc
final class AppleAuthenticationService: NSObject {

    @objc static let shared = AppleAuthenticationService()

    private struct Constants {
        static let UserAppleID = "UserAppleID"
    }

    // MARK: Variables

    @objc weak var delegate: AppleAuthenticationServiceDelegate?

    // MARK: Public

    @objc func requestAuth() {
        let appleIDProvider = ASAuthorizationAppleIDProvider()
        let request = appleIDProvider.createRequest()
        request.requestedScopes = [.fullName, .email]

        let authorizationController = ASAuthorizationController(authorizationRequests: [request])
        authorizationController.delegate = self
        authorizationController.presentationContextProvider = self
        authorizationController.performRequests()
    }

    @objc func validateCredentialState() {
        guard let userID = UserDefaults.standard.string(forKey: Constants.UserAppleID) else { return }

        // Validate again since the user can revork the permission later
        // Logout if the credential is invalid
        let appleIDProvider = ASAuthorizationAppleIDProvider()
        appleIDProvider.getCredentialState(forUserID: userID) { (credentialState, error) in
            switch credentialState {
            case .authorized:
                break // The Apple ID credential is valid.
            case .revoked, .notFound:
                // The Apple ID credential is either revoked or was not found, so show the sign-in UI.
                NotificationCenter.default.postNotificationOnMainThread(NSNotification.Name(kInvalidAppleUserCrendential), object: nil)
            default:
                break
            }
        }
    }

    @objc func reset() {
        UserDefaults.standard.removeObject(forKey: Constants.UserAppleID)
    }
}

// MARK: ASAuthorizationControllerDelegate

@available(OSX 10.15, *)
extension AppleAuthenticationService: ASAuthorizationControllerDelegate {

    func authorizationController(controller: ASAuthorizationController, didCompleteWithError error: Error) {
        delegate?.appleAuthenticationDidFailed(with: error)
    }

    func authorizationController(controller: ASAuthorizationController, didCompleteWithAuthorization authorization: ASAuthorization) {
        switch authorization.credential {
        case let appleIDCredential as ASAuthorizationAppleIDCredential:
            UserDefaults.standard.set(appleIDCredential.user, forKey: Constants.UserAppleID)
            delegate?.appleAuthenticationDidComplete(with: appleIDCredential)
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

