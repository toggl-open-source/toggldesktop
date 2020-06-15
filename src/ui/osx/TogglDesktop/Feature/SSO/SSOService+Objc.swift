//
//  SSOService+Objc.swift
//  TogglDesktop
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
