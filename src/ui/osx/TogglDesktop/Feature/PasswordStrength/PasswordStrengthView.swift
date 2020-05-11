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
        case rule // Show all password requirements
    }

    // MARK: OUTLET

    @IBOutlet weak var titleLbl: NSTextField!
    @IBOutlet weak var stackView: NSStackView!
    @IBOutlet weak var successStackView: NSStackView!
    @IBOutlet weak var successBox: NSBox!
    @IBOutlet weak var ruleBox: NSBox!

    // MARK: Variables

    private lazy var validation = PasswordStrengthValidation()
    private var widthConstraint: NSLayoutConstraint?
    private var heightConstraint: NSLayoutConstraint?
    private var displayState = DisplayState.rule {
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
        displayState = matchedRules.count == PasswordStrengthValidation.Rule.allCases.count ? .success : .rule

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
        widthConstraint = view.widthAnchor.constraint(equalToConstant: 276)
        heightConstraint = view.heightAnchor.constraint(equalToConstant: 112)
        widthConstraint?.isActive = true
        heightConstraint?.isActive = true

        // Add success rule
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

        // Prepare
        switch displayState {
        case .success:
            successBox.alphaValue = 0
            successBox.isHidden = false
            ruleBox.alphaValue = 1
            ruleBox.isHidden = true
        case .rule:
            ruleBox.alphaValue = 0
            ruleBox.isHidden = false
            successBox.alphaValue = 1
            successBox.isHidden = true
        }

        NSAnimationContext.runAnimationGroup({[weak self] (context) in
            guard let strongSelf = self else { return }
            context.duration = 0.15
            context.timingFunction = CAMediaTimingFunction(name: .easeOut)

            // Animation
            switch displayState {
            case .success:
                strongSelf.successBox.animator().alphaValue = 1
                strongSelf.ruleBox.animator().alphaValue = 0
                strongSelf.heightConstraint?.animator().constant = 48
            case .rule:
                strongSelf.successBox.animator().alphaValue = 0
                strongSelf.ruleBox.animator().alphaValue = 1
                strongSelf.heightConstraint?.animator().constant = 112
            }}, completionHandler: {[weak self] in
                guard let strongSelf = self else { return }
                switch strongSelf.displayState {
                case .success:
                    strongSelf.successBox.isHidden = false
                    strongSelf.ruleBox.isHidden = true
                case .rule:
                    strongSelf.successBox.isHidden = true
                    strongSelf.ruleBox.isHidden = false
                }
        })
    }
}
