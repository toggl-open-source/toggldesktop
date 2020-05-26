//
//  PasswordStrengthView.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 5/7/20.
//  Copyright © 2020 Alari. All rights reserved.
//

import Cocoa

final class PasswordStrengthView: NSViewController {

    struct Constants {
        static let Width: CGFloat = 276
        static let HeightRule: CGFloat = 112
    }

    // MARK: OUTLET

    @IBOutlet weak var titleLbl: NSTextField!
    @IBOutlet weak var stackView: NSStackView!
    @IBOutlet weak var ruleBox: NSBox!

    // MARK: Variables

    private lazy var validation = PasswordStrengthValidation()
    @objc var isMeetAllRequirements = false

    // MARK: View Cycle
    
    override func viewDidLoad() {
        super.viewDidLoad()
        prepareUI()
        renderRulesStackView()
    }

    // MARK: Public

    @objc func updateValidation(for text: String) {
        let matchedRules = validation.validate(text: text)
        isMeetAllRequirements = matchedRules.count == PasswordStrengthValidation.Rule.allCases.count

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
        // Prepare
        view.widthAnchor.constraint(equalToConstant: Constants.Width).isActive = true
        view.heightAnchor.constraint(equalToConstant: Constants.HeightRule).isActive = true

        // Shadow
        view.wantsLayer = true
        view.shadow = NSShadow()
        view.layer?.cornerRadius = 4.0
        view.layer?.shadowOpacity = 1.0
        view.layer?.shadowColor = NSColor(calibratedWhite: 0, alpha: 0.1).cgColor
        view.layer?.shadowOffset = NSMakeSize(0, -2)
        view.layer?.shadowRadius = 4
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
}
