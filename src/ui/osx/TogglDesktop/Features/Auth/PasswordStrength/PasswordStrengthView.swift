//
//  PasswordStrengthView.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 5/7/20.
//  Copyright © 2020 Alari. All rights reserved.
//

import Cocoa

/// Password Strength Popover View
final class PasswordStrengthView: NSViewController {

    enum DisplayState {
        case success // Show Success state
        case rule // Show all password requirements
    }

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

    /// Main func to handle the validation and present the Match/Unmatch rule
    /// - Parameter text: The evaludated text
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
        view.heightAnchor.constraint(equalToConstant: Constants.HeightRule).isActive = true

        // Shadow
        view.wantsLayer = true
        view.shadow = NSShadow()
        view.layer?.cornerRadius = 4.0
        view.layer?.shadowOpacity = 1.0
        view.layer?.shadowColor = Color.shadow.cgColor
        view.layer?.shadowOffset = NSSize(width: 0, height: -2)
        view.layer?.shadowRadius = 4
    }

    /// Map all model rules to Rule View
    private func renderRulesStackView() {
        let views = PasswordStrengthValidation.Rule.allCases.map { rule -> PasswordRuleView in
            let view = PasswordRuleView.xibView()
            view.config(with: rule, status: .none)
            return view
        }
        views.forEach {
            stackView.addArrangedSubview($0)
        }
    }

    private func updateUI() {
        let action = {
            NSAnimationContext.runAnimationGroup({[weak self] (context) in
                guard let strongSelf = self else { return }
                context.duration = 0.15
                context.timingFunction = CAMediaTimingFunction(name: .easeOut)

                // Animation
                switch strongSelf.displayState {
                case .success:
                    strongSelf.view.animator().alphaValue = 0
                case .rule:
                    strongSelf.view.animator().alphaValue = 1
                }}, completionHandler: nil)
        }

        switch displayState {
        case .success:
            // Delay 1 second before dismissing the Success view
            DispatchQueue.main.asyncAfter(deadline: .now() + .seconds(1), execute: action)
        case .rule:
            action()
        }
    }
}
