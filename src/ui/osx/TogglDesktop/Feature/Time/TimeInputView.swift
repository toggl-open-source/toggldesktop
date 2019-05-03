//
//  TimeInputView.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 5/3/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

protocol TimeInputViewDelegate: class {

    func timeInputDidSelect(_ time: TimeData, with selection: TimeInputView.Selection)
}

final class TimeInputView: NSView {

    enum Selection {
        case hour
        case minute
        case second
        case none
    }

    enum DisplayMode {
        case compact // only hour + minute
        case full // all
    }

    // MARK: OUTLET

    @IBOutlet weak var titleLbl: NSTextField!

    // MARK: Variables

    weak var delegate: TimeInputViewDelegate?
    private var time: TimeData! {
        didSet {
            renderTimeTitle()
        }
    }
    private var mode: DisplayMode = .full {
        didSet {

            // Font
            switch mode {
            case .compact:
                titleLbl.font = NSFont.systemFont(ofSize: 14.0)
            case .full:
                titleLbl.font = NSFont.systemFont(ofSize: 19.0)
            }

            // Title
            renderTimeTitle()
        }
    }

    // MARK: View cycle

    override func awakeFromNib() {
        super.awakeFromNib()
    }

    func updateLayout(with mode: DisplayMode) {
        self.mode = mode
    }

    func render(with date: Date) {
        self.time = TimeData(date: date)
    }
}

// MARK: Private

extension TimeInputView {

    fileprivate func renderTimeTitle() {
        switch mode {
        case .full:
            let text = "\(time.hour):\(time.minute):\(time.second)"
            titleLbl.stringValue = text
        case .compact:
            let text = "\(time.minute):\(time.second)"
            titleLbl.stringValue = text
        }
    }
}
