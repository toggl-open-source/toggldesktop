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

/// OnboardingViewController
final class OnboardingViewController: NSViewController {

    // MARK: OUTLET

    @IBOutlet weak var backgroundView: OnboardingBackgroundView!

    // MARK: Variable

    weak var delegate: OnboardingViewControllerDelegate?
    private var shouldClosePopover = false
    private weak var presentView: NSView?
    private lazy var contentController = OnboardingContentViewController(nibName: "OnboardingContentViewController", bundle: nil)

    /// The inner popover to present the onboard
    private lazy var popover: NSPopover = {
        let popover = NSPopover()
        popover.animates = true
        popover.behavior = .semitransient
        popover.contentViewController = contentController
        contentController.popover = popover
        popover.delegate = self
        return popover
    }()

    /// Reuse the popover from activityRecorderController
    /// Tends for recordActivity onboard
    private lazy var activityRecorderController: TimelineActivityRecorderViewController = {
        let controller = TimelineActivityRecorderViewController(nibName: "TimelineActivityRecorderViewController", bundle: nil)
        controller.delegate = self
        return controller
    }()
    private var payload: OnboardingPayload?

    /// Determine if the onboard is shown
    var isShown: Bool {
        return popover.isShown
    }

    // MARK: View Cycle

    override func viewDidLoad() {
        super.viewDidLoad()

        initCommon()
    }

    // MARK: Public

    /// Present the onboarding with given Payload
    /// - Parameter payload: OnboardingPayload
    func present(payload: OnboardingPayload) {
        self.payload = payload
        self.presentView = payload.view
        guard let presentView = presentView else { return }

        shouldClosePopover = false

        // Force render to get the correct size after autolayout
        view.setNeedsDisplay(view.bounds)
        view.displayIfNeeded()

        // Render
        if payload.fadesBackground {
            updateMaskLayer(with: presentView)
        }
        backgroundView.isHidden = !payload.fadesBackground

        contentController.config(with: payload)

        // Prepare and show
        preparePopoverContentView(with: payload)

        popover.show(relativeTo: presentView.bounds, of: presentView, preferredEdge: payload.preferEdges)
        popover.positioningRect = payload.positioningRect(payload.hint, popover.positioningRect)
    }

    /// Dismiss all onboard view
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

        // Observe the window size to redraw the mask view
        NotificationCenter.default.addObserver(self,
                                               selector: #selector(self.windowDidResizeNoti(_:)),
                                               name: NSWindow.didResizeNotification,
                                               object: nil)
    }

    /// Present the Onboarding view
    /// - Parameter payload: OnboardingPayload
    private func preparePopoverContentView(with payload: OnboardingPayload) {
        switch payload.hint {
        case .recordActivity:
            popover.contentViewController = activityRecorderController // Reuse Activity Popover
        default:
            popover.contentViewController = contentController
        }

        // Update size
        if let size = popover.contentViewController?.view.frame.size {
            popover.contentSize = size
        }
    }

    @objc private func windowDidResizeNoti(_ noti: Notification) {
        guard let window = noti.object as? NSWindow,
            window === self.view.window else { return }
        guard let presentView = presentView, popover.isShown else { return }

        // Re-draw the mask layer if the window size is changed
        updateMaskLayer(with: presentView)
    }

    /// Re-draw the mask view to fit with new window's size
    /// - Parameter hintView: Current Hint View
    private func updateMaskLayer(with hintView: NSView) {
        let hintFrame = view.convert(hintView.frame, from: hintView.superview)
        backgroundView.drawMask(at: hintFrame)
    }
}

// MARK: NSPopoverDelegate

extension OnboardingViewController: NSPopoverDelegate {

    func popoverShouldClose(_ popover: NSPopover) -> Bool {
        if payload?.fadesBackground == true {
            // if dimming background is visible we don't close popup till user clicks on background
            return shouldClosePopover
        }
        return true
    }

    func popoverWillClose(_ notification: Notification) {
        delegate?.onboardingViewControllerDidClose()
    }
}

// MARK: OnboardingBackgroundViewDelegate

extension OnboardingViewController: OnboardingBackgroundViewDelegate {

    func onboardingBackgroundDidClick(_ sender: OnboardingBackgroundView, onHighlightedArea: Bool, event: NSEvent) {
        //
        // Manual trigger the close when the user click on the background
        // Because some view that NSPopover presents would be re-cycle (Ex: TimeEntryCell when the Collection View reload)
        // It causes the Popover unexpectedly close in a second after presenting
        //
        shouldClosePopover = true
        popover.close()

        // As the TimeEntry Collection View handle the click differently
        // So we have to pass the Click Event to the CollectionView
        // and present the TimeEntry Editor if need
        if let payload = payload,
            payload.hint == .editTimeEntry && onHighlightedArea {
            NotificationCenter.default.post(name: .onboardingDidClick, object: event)
        }
    }
}

// MARK: TimelineActivityRecorderViewControllerDelegate

extension OnboardingViewController: TimelineActivityRecorderViewControllerDelegate {

    func timelineActivityRecorderShouldDidClickOnCloseBtn(_ sender: Any) {
        popover.close()
    }
}
