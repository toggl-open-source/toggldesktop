//
//  OnboardingContentViewController.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 4/1/20.
//  Copyright © 2020 Alari. All rights reserved.
//

import Cocoa

final class OnboardingContentViewController: NSViewController {

    // MARK: OUTLET

    @IBOutlet weak var titleTextField: NSTextField!
    weak var popover: NSPopover?

    // MARK: Public

    func config(with payload: OnboardingPayload) {
        self.titleTextField.stringValue = payload.title
    }

    @IBAction func exitBtnOnClick(_ sender: Any) {
        popover?.close()
    }
}
