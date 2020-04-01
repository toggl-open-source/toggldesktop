//
//  OnboardingViewController.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 4/1/20.
//  Copyright © 2020 Alari. All rights reserved.
//

import Cocoa

final class OnboardingViewController: NSViewController {

    // MARK: OUTLET

    @IBOutlet weak var containerView: NSView!

    // MARK: Variable

    private lazy var contentController: OnboardingContentViewController = OnboardingContentViewController(nibName: "OnboardingContentViewController", bundle: nil)
    private lazy var popover: NoVibrantPopoverView = {
        let popover = NoVibrantPopoverView()
        popover.animates = true
        popover.behavior = .semitransient
        popover.contentViewController = contentController
        return popover
    }()

    // MARK: View Cycle

    override func viewDidLoad() {
        super.viewDidLoad()

    }
    
}
