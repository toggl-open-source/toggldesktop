//
//  PasswordStrengthView.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 5/7/20.
//  Copyright © 2020 Alari. All rights reserved.
//

import Cocoa

final class PasswordStrengthView: NSViewController {

    // MARK: OUTLET

    @IBOutlet weak var titleLbl: NSTextField!
    @IBOutlet weak var stackView: NSStackView!

    // MARK: Variables

    private lazy var validation = PasswordStrengthValidation()

    // MARK: View Cycle
    
    override func viewDidLoad() {
        super.viewDidLoad()
        renderRulesStackView()
    }

    // MARK: Public

    func updateValidation(for text: String) {
        let matchedRules = validation.validate(text: text)
        stackView.arrangedSubviews.forEach { (view) in
            guard let ruleView = view as? PasswordRuleView,
                let rule = ruleView.rule else { return }
            if text.isEmpty {
                ruleView.updateStatus(.none)
            } else {
                let status: PasswordStrengthValidation.MatchStatus = matchedRules.contains(rule) ? .match : .unmatch
                ruleView.updateStatus(status)
            }
        }
    }
}

// MARK: Private

extension PasswordStrengthView {

    private func renderRulesStackView() {
        let views = PasswordStrengthValidation.Rule.allCases.map { rule -> PasswordRuleView in
            let view = PasswordRuleView.xibView() as PasswordRuleView
            view.config(with: rule, status: .none)
            return view
        }
        views.forEach {
            stackView.addArrangedSubview($0)
        }
    }
}
