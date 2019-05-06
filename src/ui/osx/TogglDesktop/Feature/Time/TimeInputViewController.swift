//
//  TimeInputViewController.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 5/3/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

final class TimeInputViewController: NSViewController {

    // MARK: OUTLET

    @IBOutlet weak var durationContainerView: NSView!
    @IBOutlet weak var timeStackView: NSStackView!
    @IBOutlet weak var clockContainerView: NSView!
    @IBOutlet weak var durationLbl: NSTextField!
    
    // MARK: Variables

    private var isAppearing = false
    private var timeEntry: TimeEntryViewItem! {
        didSet {
            if isAppearing {
                layoutContent()
            }
        }

    }
    // MARK: View Cycle

    override func viewDidLoad() {
        super.viewDidLoad()

        initCommon()
        setupTimeInputs()
        layoutContent()
    }

    override func viewDidAppear() {
        super.viewDidAppear()
        isAppearing = true
    }

    override func viewWillDisappear() {
        super.viewWillDisappear()
        isAppearing = false
    }

    private func layoutContent() {

    }
}

// MARK: Private

extension TimeInputViewController {

    fileprivate func initCommon() {

    }

    fileprivate func setupTimeInputs() {

        // Duration
        let durationInputView = TimeInputView.xibView() as TimeInputView
        durationInputView.translatesAutoresizingMaskIntoConstraints = false
        durationInputView.addSubview(durationInputView)
        durationInputView.topAnchor.constraint(equalTo: durationLbl.bottomAnchor, constant: 10).isActive = true
        durationInputView.leftAnchor.constraint(equalTo: durationContainerView.leftAnchor, constant: 20).isActive = true

        // Start
    }
}
