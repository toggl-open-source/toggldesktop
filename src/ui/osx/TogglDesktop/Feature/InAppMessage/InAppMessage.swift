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
    let subTitle: String
    let buttonTitle: String
    let urlAction: String

    @objc
    init(title: String, subTitle: String, buttonTitle: String, urlAction: String) {
        self.title = title
        self.subTitle = subTitle
        self.buttonTitle = buttonTitle
        self.urlAction = urlAction
    }
}
