//
//  SSOService+Objc.swift
//  TogglTrack
//
//  Created by Nghia Tran on 6/10/20.
//  Copyright © 2020 Alari. All rights reserved.
//

import Foundation

/// Expose all SSO Service swift class to Objc world
@objc final class SSOServiceObjc: NSObject {

    @objc class func handleCallback(with url: URL) {
        SSOService.shared.handleCallback(with: url)
    }

    @objc class func authorize(with urlStr: String) {
        SSOService.shared.authorize(with: urlStr)
    }
}

@objcMembers
class SSOPayload: NSObject {

    let email: String
    let confirmationCode: String

    init(email: String, confirmationCode: String) {
        self.email = email
        self.confirmationCode = confirmationCode
        super.init()
    }
}
