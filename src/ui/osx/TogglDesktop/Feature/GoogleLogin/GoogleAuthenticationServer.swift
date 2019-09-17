//
//  GoogleAuthenticationServer.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 9/17/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation
import AppAuth
import GTMAppAuth

final class GoogleAuthenticationServer {

    static let shared = GoogleAuthenticationServer()

    private struct Constants {
        static let Issuer = "https://accounts.google.com"
        static let ClientID = "426090949585-uj7lka2mtanjgd7j9i6c4ik091rcv6n5.apps.googleusercontent.com"
        static let ClientSecret = "6IHWKIfTAMF7cPJsBvoGxYui"
        static let RedirectURI = "com.googleusercontent.apps.toggl:/oauthredirect"
        static let TogglAuthorizerKey = "toggldesktop-authorization"
    }

    struct GoogleUser {

        let accessToken: String
        let email: String
    }

    // MARK: Variables

    init() {
        
    }

    // MARK: Public

    func authenticate(_ complete: (GoogleUser?, Error?) -> Void) {

    }

}

// MARK: Private

extension GoogleAuthenticationServer {

    fileprivate func discoverService() {
        
    }
}
