//
//  GoogleAuthenticationServer+Objc.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 9/17/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

@objc class GoogleAuthenticationServerHelper: NSObject {

    @objc class func authorize() {
        GoogleAuthenticationServer.shared.authenticate { (user, error) in

        }
    }
}
