//
//  InAppMessageViewController.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 12/3/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

protocol InAppMessageViewControllerDelegate: class {

    func InAppMessageViewControllerShouldDismiss()
}

final class InAppMessageViewController: NSViewController {

    // MARK: Variables

    @IBOutlet weak var containerView: NSView!
    @IBOutlet weak var titleLbl: NSTextField!
    @IBOutlet weak var descriptionLbl: NSTextField!
    @IBOutlet weak var actionBtn: NSButton!

    // MARK: Variables

    weak var delegate: InAppMessageViewControllerDelegate?

    // MARK: View

    override func viewDidLoad() {
        super.viewDidLoad()
        // Do view setup here.
    }

    @objc class func initFromXib() -> InAppMessageViewController {
        return InAppMessageViewController.xibView()
    }

    @IBAction func closeBtnOnTap(_ sender: Any) {
        
    }

    @IBAction func actionBtnOnTap(_ sender: Any) {
        
    }

    func update(message: InAppMessage) {
        
    }
}

// MARK: Private

extension InAppMessageViewController {

}
