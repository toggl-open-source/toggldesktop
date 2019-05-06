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
    @IBOutlet weak var startLbl: NSTextField!
    @IBOutlet weak var startContainerView: NSView!
    @IBOutlet weak var endLbl: NSTextField!
    @IBOutlet weak var endContainerView: NSView!

    // MARK: Variables

    private var isAppearing = false
    var timeEntry: TimeEntryViewItem! {
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
        durationInputView.updateLayout(with: .full)
        durationInputView.translatesAutoresizingMaskIntoConstraints = false
        durationContainerView.addSubview(durationInputView)
        durationInputView.topAnchor.constraint(equalTo: durationLbl.bottomAnchor, constant: 10).isActive = true
        durationInputView.leftAnchor.constraint(equalTo: durationContainerView.leftAnchor, constant: 20).isActive = true

        // Start
        let startInputView = TimeInputView.xibView() as TimeInputView
        startInputView.updateLayout(with: .compact)
        startInputView.translatesAutoresizingMaskIntoConstraints = false
        startContainerView.addSubview(startInputView)
        startInputView.topAnchor.constraint(equalTo: startLbl.bottomAnchor, constant: 0).isActive = true
        startInputView.leftAnchor.constraint(equalTo: startContainerView.leftAnchor, constant: 10).isActive = true

        // End
        let endInputView = TimeInputView.xibView() as TimeInputView
        endInputView.updateLayout(with: .compact)
        endInputView.translatesAutoresizingMaskIntoConstraints = false
        endContainerView.addSubview(endInputView)
        endInputView.topAnchor.constraint(equalTo: endLbl.bottomAnchor, constant: 0).isActive = true
        endInputView.leftAnchor.constraint(equalTo: endContainerView.leftAnchor, constant: 10).isActive = true
    }
}
