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
    private var shouldClosePopover = false
    private weak var presentView: NSView?
    private lazy var contentController: OnboardingContentViewController = OnboardingContentViewController(nibName: "OnboardingContentViewController", bundle: nil)
    private lazy var popover: NSPopover = {
        let popover = NSPopover()
        popover.animates = true
        popover.behavior = .applicationDefined // Manual trigger the close action
        popover.contentViewController = contentController
        contentController.popover = popover
        popover.delegate = self
        return popover
    }()

    var isShown: Bool {
        return popover.isShown
    }

    // MARK: View Cycle

    override func viewDidLoad() {
        super.viewDidLoad()

        initCommon()
    }

    deinit {
        NotificationCenter.default.removeObserver(self)
    }

    // MARK: Public

    func present(payload: OnboardingPayload) {
        self.presentView = payload.view
        guard let presentView = presentView else { return }

        shouldClosePopover = false

        // Force render to get the correct size after autolayout
        view.setNeedsDisplay(view.bounds)
        view.displayIfNeeded()

        // Render
        updateMaskLayer(with: presentView)
        contentController.config(with: payload)
        popover.show(relativeTo: payload.bounds, of: presentView, preferredEdge: payload.preferEdges)
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
        _ = contentController.view
        backgroundView.delegate = self
        NotificationCenter.default.addObserver(self,
                                               selector: #selector(self.windowDidResizeNoti(_:)),
                                               name: NSWindow.didResizeNotification,
                                               object: nil)
    }

    @objc private func windowDidResizeNoti(_ noti: Notification) {
        guard let window = noti.object as? NSWindow,
            window === self.view.window else { return }
        guard let presentView = presentView, popover.isShown else { return }

        // Re-draw the mask layer if the window size is changed
        updateMaskLayer(with: presentView)
    }

    private func updateMaskLayer(with hintView: NSView) {
        let hintFrame = view.convert(hintView.frame, from: hintView.superview)
        backgroundView.drawMask(at: hintFrame)
    }
}

// MARK: NSPopoverDelegate

extension OnboardingViewController: NSPopoverDelegate {

    func popoverShouldClose(_ popover: NSPopover) -> Bool {
        return shouldClosePopover
    }
    
    func popoverWillClose(_ notification: Notification) {
        delegate?.onboardingViewControllerDidClose()
    }
}

// MARK: OnboardingBackgroundViewDelegate

extension OnboardingViewController: OnboardingBackgroundViewDelegate {

    func onboardingBackgroundDidClick(_ sender: OnboardingBackgroundView) {
        //
        // Manual trigger the close when the user click on the background
        // Because some view that NSPopover presents would be re-cycle (Ex: TimeEntryCell when the Collection View reload)
        // It causes the Popover unexpectedly close in a second after presenting
        //
        shouldClosePopover = true
        popover.close()
    }
}
