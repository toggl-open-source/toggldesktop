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
    private weak var hintView: NSView?
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

    deinit {
        NotificationCenter.default.removeObserver(self)
    }

    // MARK: Public

    func present(payload: OnboardingPayload, hintView: NSView) {
        self.hintView = hintView

        // Force render to get the correct size after autolayout
        view.setNeedsDisplay(view.bounds)
        view.displayIfNeeded()

        // Render
        updateMaskLayer(with: hintView)
        contentController.config(with: payload)
        popover.present(from: hintView.bounds, of: hintView, preferredEdge: payload.preferEdges)
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
        NotificationCenter.default.addObserver(self,
                                               selector: #selector(self.windowDidResizeNoti(_:)),
                                               name: NSWindow.didResizeNotification,
                                               object: nil)
    }

    @objc private func windowDidResizeNoti(_ noti: Notification) {
        guard let window = noti.object as? NSWindow,
            window === self.view.window else { return }
        guard let hintView = hintView, popover.isShown else { return }

        // Re-draw the mask layer if the window size is changed
        updateMaskLayer(with: hintView)
    }

    private func updateMaskLayer(with hintView: NSView) {
        let hintFrame = view.convert(hintView.frame, from: hintView.superview)
        backgroundView.drawMask(at: hintFrame)
    }
}

// MARK: NSPopoverDelegate

extension OnboardingViewController: NSPopoverDelegate {

    func popoverWillClose(_ notification: Notification) {
        delegate?.onboardingViewControllerDidClose()
    }
}
