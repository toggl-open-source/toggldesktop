//
//  TimeEntryEmptyView.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 3/14/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

final class TimeEntryEmptyView: NSView {

    fileprivate struct Constants {

        static let stopwatchImage = NSImage.Name("")
        static let spiderImage = NSImage.Name("")
    }

    enum DisplayMode {
        case welcome
        case noEntry
    }

    // MARK: Variables

    private var displayMode = DisplayMode.welcome {
        didSet {
            layoutView()
        }
    }

    // MARK: OUTLET

    @IBOutlet weak var iconImageView: NSImageView!
    @IBOutlet weak var titleLabel: NSTextField!
    @IBOutlet weak var subTitleLabel: NSTextField!
    @IBOutlet weak var loadMoreBtn: NSButton!


    // MARK: Public

    func setDisplayMode(_ mode: DisplayMode) {
        displayMode = mode
    }

    @IBAction func loadMoreOnTap(_ sender: Any) {

    }
}

// MARK: Private

extension TimeEntryEmptyView {

    fileprivate func layoutView() {
        switch displayMode {
        case .welcome:
            iconImageView.image = NSImage(named: Constants.stopwatchImage)
            titleLabel.stringValue = "Welcome to Toggl"
            titleLabel.stringValue = "Time each activity you do and see where your hours go"
        case .noEntry:
            iconImageView.image = NSImage(named: Constants.spiderImage)
            titleLabel.stringValue = "No recent entries"
            titleLabel.stringValue = "It’s been a long time since you’ve tracked your tasks!"
        }
    }
}
