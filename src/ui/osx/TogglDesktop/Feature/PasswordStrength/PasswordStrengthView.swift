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

    struct Constants {
        static let Width: CGFloat = 276
        static let HeightSuccess: CGFloat = 48
        static let HeightRule: CGFloat = 112
    }

    // MARK: OUTLET

    @IBOutlet weak var titleLbl: NSTextField!
    @IBOutlet weak var stackView: NSStackView!
    @IBOutlet weak var successStackView: NSStackView!
    @IBOutlet weak var successBox: NSBox!
    @IBOutlet weak var ruleBox: NSBox!

    // MARK: Variables

    private lazy var validation = PasswordStrengthValidation()
    private var heightConstraint: NSLayoutConstraint?
    private var displayState = DisplayState.rule {
        didSet {
            guard displayState != oldValue else { return }
            updateUI()
        }
    }

    @objc var isMeetAllRequirements: Bool {
        return displayState == .success
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
        view.widthAnchor.constraint(equalToConstant: Constants.Width).isActive = true
        heightConstraint = view.heightAnchor.constraint(equalToConstant: Constants.HeightRule)
        heightConstraint?.isActive = true

        // Add success rule
        let successView = PasswordRuleView.xibView() as PasswordRuleView
        successView.updateSuccessStatus()
        successStackView.addArrangedSubview(successView)

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
                strongSelf.heightConstraint?.animator().constant = Constants.HeightSuccess
            case .rule:
                strongSelf.successBox.animator().alphaValue = 0
                strongSelf.ruleBox.animator().alphaValue = 1
                strongSelf.heightConstraint?.animator().constant = Constants.HeightRule
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
