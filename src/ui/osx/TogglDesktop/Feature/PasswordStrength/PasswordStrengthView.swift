//
//  PasswordStrengthView.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 5/7/20.
//  Copyright © 2020 Alari. All rights reserved.
//

import Cocoa

final class PasswordStrengthView: NSViewController {

    enum DisplayState {
        case success // Show Success state
        case hasIssues // Show all password requirements
    }

    // MARK: OUTLET

    @IBOutlet weak var titleLbl: NSTextField!
    @IBOutlet weak var stackView: NSStackView!
    @IBOutlet weak var successStackView: NSStackView!
    @IBOutlet weak var successBox: NSBox!
    @IBOutlet weak var ruleBox: NSBox!

    // MARK: Variables

    private lazy var validation = PasswordStrengthValidation()
    private var displayState = DisplayState.hasIssues {
        didSet {
            guard displayState != oldValue else { return }
            updateUI()
        }
    }

    // MARK: View Cycle
    
    override func viewDidLoad() {
        super.viewDidLoad()
        prepareUI()
        renderRulesStackView()
    }

    // MARK: Public

    @objc func updateValidation(for text: String) {
        let matchedRules = validation.validate(text: text)

        // If all rules are matched -> Update the UI
        displayState = matchedRules.count == PasswordStrengthValidation.Rule.allCases.count ? .success : .hasIssues

        // Update individual state
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

    private func prepareUI() {
        let view = PasswordRuleView.xibView() as PasswordRuleView
        view.updateSuccessStatus()
        successStackView.addArrangedSubview(view)
    }

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

    private func updateUI() {
        switch displayState {
        case .success:
            successBox.isHidden = false
            ruleBox.isHidden = true
        case .hasIssues:
            successBox.isHidden = true
            ruleBox.isHidden = false
        }
    }
}
