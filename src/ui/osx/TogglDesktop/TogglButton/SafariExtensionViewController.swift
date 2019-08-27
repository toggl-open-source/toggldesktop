//
//  SafariExtensionViewController.swift
//  TogglButton
//
//  Created by Nghia Tran on 8/27/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import SafariServices

class SafariExtensionViewController: SFSafariExtensionViewController {
    
    static let shared: SafariExtensionViewController = {
        let shared = SafariExtensionViewController()
        shared.preferredContentSize = NSSize(width:320, height:240)
        return shared
    }()

}
