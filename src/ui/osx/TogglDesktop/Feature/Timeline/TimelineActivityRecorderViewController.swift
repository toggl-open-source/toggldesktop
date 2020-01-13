//
//  TimelineActivityRecorderViewController.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 1/3/20.
//  Copyright © 2020 Alari. All rights reserved.
//

import Cocoa

protocol TimelineActivityRecorderViewControllerDelegate: class {

    func timelineActivityRecorderShouldDidClickOnCloseBtn(_ sender: Any)
}

final class TimelineActivityRecorderViewController: LayerBackedViewController {

    private struct Constants {
        static let LearnMoreURL = "https://toggl.com"
    }

    // MARK: OUTLET

    @IBOutlet weak var titleLbl: NSTextField!
    @IBOutlet weak var descriptionLbl: NSTextField!

    // MARK: Variables

    weak var delegate: TimelineActivityRecorderViewControllerDelegate?

    // MARK: View

    override func viewDidLoad() {
        super.viewDidLoad()
    }

    // MARK: Public

    @IBAction func learnMoreBtnOnTap(_ sender: Any) {
        NSWorkspace.shared.open(URL(string: Constants.LearnMoreURL)!)
    }

    @IBAction func closeBtnOnTap(_ sender: Any) {
        delegate?.timelineActivityRecorderShouldDidClickOnCloseBtn(self)
    }

}
