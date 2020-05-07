//
//  PasswordStrengthValidation.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 5/7/20.
//  Copyright © 2020 Alari. All rights reserved.
//

import Foundation

final class PasswordStrengthValidation {

    enum Rule: CaseIterable {
        case moreThanEightLetters
        case lowerAndUpperCases
        case atLeastOneNumber

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

    // MARK: Variables

    static let `default` = PasswordStrengthValidation()
    private let rules: [Rule]

    // MARK: Init

    init(rules: [Rule] = Rule.allCases) {
        self.rules = rules
    }

    // MARK: Public

    func validate(text: String) -> [Rule] {
        // Return all matched rules
        return rules.compactMap { rule -> Rule? in
            return rule.validate(with: text) ? rule : nil
        }
    }
}
