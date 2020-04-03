//
//  OnboardingViewController.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 4/1/20.
//  Copyright © 2020 Alari. All rights reserved.
//

import Cocoa

protocol OnboardingViewControllerDelegate: class {

    func onboardingViewControllerDidClose()
}

final class OnboardingViewController: NSViewController {

    // MARK: OUTLET

    @IBOutlet weak var backgroundView: OnboardingBackgroundView!

    // MARK: Variable

    weak var delegate: OnboardingViewControllerDelegate?
    private lazy var contentController: OnboardingContentViewController = OnboardingContentViewController(nibName: "OnboardingContentViewController", bundle: nil)
    private lazy var popover: NoVibrantPopoverView = {
        let popover = NoVibrantPopoverView()
        popover.animates = true
        popover.behavior = .semitransient
        popover.contentViewController = contentController
        contentController.popover = popover
        popover.delegate = self
        return popover
    }()

    // MARK: View Cycle

    override func viewDidLoad() {
        super.viewDidLoad()

        initCommon()
    }

    // MARK: Public

    func present(payload: OnboardingPayload, view: NSView) {
        backgroundView.setMaskPosition(at: view)
        contentController.config(with: payload)
        popover.present(from: view.bounds, of: view, preferredEdge: payload.preferEdges)
    }

    func dismiss() {
        popover.close()
    }
}

// MARK: Private

extension OnboardingViewController {

    private func initCommon() {
        // Pre-load the view
        _ = popover
    }
}

// MARK: NSPopoverDelegate

extension OnboardingViewController: NSPopoverDelegate {

    func popoverWillClose(_ notification: Notification) {
        delegate?.onboardingViewControllerDidClose()
    }
}
