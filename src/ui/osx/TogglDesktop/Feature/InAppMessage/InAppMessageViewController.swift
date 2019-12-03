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
    @IBOutlet weak var actionBtn: FlatButton!

    // MARK: Variables

    weak var delegate: InAppMessageViewControllerDelegate?
    private var message: InAppMessage?

    // MARK: View

    override func viewDidLoad() {
        super.viewDidLoad()
        // Do view setup here.
    }

    @objc class func initFromXib() -> InAppMessageViewController {
        return InAppMessageViewController.xibView()
    }

    @IBAction func closeBtnOnTap(_ sender: Any) {
        DesktopLibraryBridge.shared().setSeenInAppMessageWithID("0")
        delegate?.InAppMessageViewControllerShouldDismiss()
    }

    @IBAction func actionBtnOnTap(_ sender: Any) {
        guard let message = message,
            let url = URL(string: message.urlAction) else { return }
        NSWorkspace.shared.open(url)
    }

    func update(_ message: InAppMessage) {
        self.message = message
        titleLbl.stringValue = message.title
        descriptionLbl.stringValue = message.subTitle
        actionBtn.title = message.buttonTitle
    }
}

// MARK: Private

extension InAppMessageViewController {

    private func initCommon() {
        actionBtn.wantsLayer = true
        actionBtn.layer?.borderWidth = 1
        actionBtn.layer?.borderColor = NSColor.white.cgColor
    }
}
