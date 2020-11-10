//
//  PasswordRuleView.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 5/7/20.
//  Copyright © 2020 Alari. All rights reserved.
//

import Cocoa

/// Represent a single Rule View
final class PasswordRuleView: NSView {

    // MARK: Variables

    @IBOutlet weak var titleLbl: NSTextField!
    @IBOutlet weak var iconImageView: NSImageView!

    // MARK: Variable

    private(set) var rule: PasswordStrengthValidation.Rule?

    // MARK: Public

    /// Update UI with given rule
    /// - Parameters:
    ///   - rule: Password Rule
    ///   - status: Status
    func config(with rule: PasswordStrengthValidation.Rule, status: PasswordStrengthValidation.MatchStatus) {
        self.rule = rule
        titleLbl.stringValue = rule.title
        updateStatus(status)
    }

    /// Update UI for current rule
    /// - Parameter status: A new status
    func updateStatus(_ status: PasswordStrengthValidation.MatchStatus) {
        titleLbl.textColor = getTextColor(for: status)
        iconImageView.image = getIconImageView(for: status)
    }

    /// Get proper color for given status
    /// - Parameter status: Match status
    /// - Returns: Text Color
    private func getTextColor(for status: PasswordStrengthValidation.MatchStatus) -> NSColor {
        switch status {
        case .match:
            return Color.green.color
        case .unmatch:
            return Color.errorTitle.color
        case .none:
            return Color.greyText.color
        }
    }

    /// Get proper Icon for given status
    /// - Parameter status: Password Status
    /// - Returns: An Image
    private func getIconImageView(for status: PasswordStrengthValidation.MatchStatus) -> NSImage {
        switch status {
        case .match:
            return NSImage(named: "password_green_check")!
        case .unmatch:
            return NSImage(named: NSImage.statusUnavailableName)!
        case .none:
            return NSImage(named: NSImage.statusNoneName)!
        }
    }
}
