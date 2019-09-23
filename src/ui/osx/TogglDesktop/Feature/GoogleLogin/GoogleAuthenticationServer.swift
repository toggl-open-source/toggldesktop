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
        static let RedirectURI = "com.googleusercontent.apps.426090949585-uj7lka2mtanjgd7j9i6c4ik091rcv6n5:/oauthredirect"
        static let TogglAuthorizerKey = "toggldesktop-authorization"
        static let Scopes = [OIDScopeOpenID, OIDScopeProfile, OIDScopeEmail]
    }

    enum GoogleError: Error, LocalizedError {
        case invalidIssuerURL
        case invalidRedirectURL
        case missingConfig
        case missingAccessToken
        case custom(Error?)

        var errorDescription: String? {
            switch self {
            case .invalidIssuerURL:
                return "Invalid issuer URL"
            case .invalidRedirectURL:
                return "Invalid Redirect URL"
            case .missingConfig:
                return "Missing config from Google"
            case .missingAccessToken:
                return "Missing Access Token"
            case .custom(let error):
                return "\(String(describing: error))"
            }
        }
    }

    struct GoogleUser {
        let accessToken: String
    }

    // MARK: Variables

    init() {
        
    }

    // MARK: Public

    func authenticate(_ complete: @escaping (GoogleUser?, GoogleError?) -> Void) {
        discoverService {[weak self] (config, error) in
            guard let strongSelf = self else { return }

            if let error = error {
                complete(nil, error)
                return
            }

            guard let config = config else {
                complete(nil, .missingConfig)
                return
            }

            // Authenticate with google
            strongSelf.makeAuthenticationRequest(with: config, complete: complete)
        }
    }
}

// MARK: Private

extension GoogleAuthenticationServer {

    fileprivate func discoverService(_ complete: @escaping (OIDServiceConfiguration?, GoogleError?) -> Void) {

        // Double check the URL
        guard let issuerURL = URL(string: Constants.Issuer) else {
            complete(nil, .invalidIssuerURL)
            return
        }

        // Get service
        OIDAuthorizationService.discoverConfiguration(forIssuer: issuerURL) { (config, error) in
            if let error = error {
                complete(nil, .custom(error))
                return
            }

            complete(config, nil)
        }
    }

    fileprivate func makeAuthenticationRequest(with config: OIDServiceConfiguration, complete: @escaping (GoogleUser?, GoogleError?) -> Void) {
        guard let redirectURL = URL(string: Constants.RedirectURI) else {
            complete(nil, .invalidRedirectURL)
            return
        }

        // Request to service
        let request = OIDAuthorizationRequest(configuration: config, clientId: Constants.ClientID,
                                              clientSecret: Constants.ClientSecret,
                                              scopes: Constants.Scopes,
                                              redirectURL: redirectURL,
                                              responseType: OIDResponseTypeCode,
                                              additionalParameters: nil)

        // Wait callback URL
        let appDelegate = NSApp.delegate as! AppDelegate
        appDelegate.currentAuthorizationFlow = OIDAuthState.authState(byPresenting: request, callback: { (authState, error) in
            if let error = error {
                complete(nil, .custom(error))
                return
            }

            // Process auth state
            if let authState = authState {
                if let token = authState.lastTokenResponse?.accessToken {
                    complete(GoogleUser(accessToken: token), nil)
                    return
                }
            }

            // Otherwise
            complete(nil, .missingAccessToken)
        })
    }
}
