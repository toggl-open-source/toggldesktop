//
//  OnboardingService.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 4/1/20.
//  Copyright © 2020 Alari. All rights reserved.
//

import Foundation

final class OnboardingService {

    static let shared = OnboardingService()

    // MARK: Variables

    private lazy var controller: OnboardingViewController = OnboardingViewController(nibName: "OnboardingViewController", bundle: nil)
    var isShown: Bool {
        return controller.isShown
    }
    
    // MARK: Init

    private init() {
        controller.delegate = self
    }

    // MARK: Public

    func present(hint: OnboardingHint, view: NSView) {
        guard let windowContentView = view.window?.contentView else { return }

        // Prevent crash if it's already added to the view hierarchy
        guard controller.view.superview == nil else { return }

        // Add to entire windows
        windowContentView.addSubview(controller.view)
        controller.view.edgesToSuperView()

        // Present
        let payload = OnboardingPayload(hint: hint)
        controller.present(payload: payload, hintView: view)
    }

    func dismiss() {
        guard controller.view.superview != nil else { return }
        controller.dismiss()
    }
}

// MARK: OnboardingViewControllerDelegate

extension OnboardingService: OnboardingViewControllerDelegate {

    func onboardingViewControllerDidClose() {
        controller.view.removeFromSuperview()
    }
}
