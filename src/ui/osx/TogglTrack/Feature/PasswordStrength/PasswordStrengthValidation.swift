//
//  PasswordStrengthValidation.swift
//  TogglTrack
//
//  Created by Nghia Tran on 5/7/20.
//  Copyright © 2020 Alari. All rights reserved.
//

import Foundation

/// Responsible for
final class PasswordStrengthValidation {

    /// Rule types
    enum Rule: CaseIterable {
        case moreThanEightLetters
        case lowerAndUpperCases
        case atLeastOneNumber

        /// String for UI
        var title: String {
            switch self {
            case .atLeastOneNumber: return "at least one number"
            case .lowerAndUpperCases: return "lowercase and uppercase letters"
            case .moreThanEightLetters: return "8 or more characters"
            }
        }

        /// Determine if the text is match with given rule
        /// - Parameter text: Raw Text
        /// - Returns: Boolean determine if it's matched
        func validate(with text: String) -> Bool {
            switch self {
            case .moreThanEightLetters:
                return text.count >= 8
            case .atLeastOneNumber:
                return text.rangeOfCharacter(from: .decimalDigits) != nil
            case .lowerAndUpperCases:
                return text.rangeOfCharacter(from: .lowercaseLetters) != nil &&
                    text.rangeOfCharacter(from: .uppercaseLetters) != nil
            }
        }
    }

    /// Status
    enum MatchStatus {
        case match
        case unmatch
        case none
    }

    // MARK: Variables

    static let `default` = PasswordStrengthValidation()
    private let rules: [Rule]

    // MARK: Init

    init(rules: [Rule] = Rule.allCases) {
        self.rules = rules
    }

    // MARK: Public

    /// Find matched rules with given text
    /// - Parameter text: User's password
    /// - Returns: the matched rules
    func validate(text: String) -> [Rule] {
        // Return all matched rules
        return rules.compactMap { rule -> Rule? in
            return rule.validate(with: text) ? rule : nil
        }
    }
}
