//
//  InAppMessage.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 12/3/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

@objcMembers
final class InAppMessage: NSObject {

    let title: String
    let desciption: String
    let buttonTitle: String
    let urlAction: String

    @objc
    init(title: String, desciption: String, buttonTitle: String, urlAction: String) {
        self.title = title
        self.desciption = desciption
        self.buttonTitle = buttonTitle
        self.urlAction = urlAction
    }
}
