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

    // MARK: Public

    func present(hint: OnboardingHint, view: NSView) {
        guard let windowContentView = view.window?.contentView else { return }

        // Prevent crash if it's already added to the view hierarchy
        guard controller.view.superview == nil else { return}

        // Add to entire windows
        windowContentView.addSubview(controller.view)
        controller.view.edgesToSuperView()

        // Present
        let payload = OnboardingPayload(hint: hint)
        controller.present(payload: payload, view: view)
    }

    func dismiss() {
        controller.removeFromParent()
        controller.dismiss()
    }
}
