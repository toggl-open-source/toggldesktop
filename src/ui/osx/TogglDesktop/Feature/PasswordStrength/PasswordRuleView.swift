//
//  PasswordRuleView.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 5/7/20.
//  Copyright © 2020 Alari. All rights reserved.
//

import Cocoa

final class PasswordRuleView: NSView {

    // MARK: Variables

    @IBOutlet weak var titleLbl: NSTextField!
    @IBOutlet weak var iconImageView: NSImageView!

    // MARK: Public

    func config(with rule: PasswordStrengthValidation.Rule) {
        titleLbl.stringValue = rule.title
    }
}
