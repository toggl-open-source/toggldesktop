//
//  AppleAuthenticationService.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 3/3/20.
//  Copyright © 2020 Alari. All rights reserved.
//

import Foundation
import AuthenticationServices

protocol AppleAuthenticationServiceDelegate: class {

}

final class AppleAuthenticationService {

    static let shared = AppleAuthenticationService()

    // MARK: Variables

    weak var delegate: AppleAuthenticationServiceDelegate?

    // MARK: Public

    func requestAppleAuthorization() {
                if #available(OSX 10.15, *) {
                    let appleIDProvider = ASAuthorizationAppleIDProvider()
                    let request = appleIDProvider.createRequest()
                    request.requestedScopes = [.fullName, .email]

                    let authorizationController = ASAuthorizationController(authorizationRequests: [request])
        //            authorizationController.delegate = self
        //            authorizationController.presentationContextProvider = self
                    authorizationController.performRequests()
                } else {
                    // Fallback on earlier versions
                }
    }
}
