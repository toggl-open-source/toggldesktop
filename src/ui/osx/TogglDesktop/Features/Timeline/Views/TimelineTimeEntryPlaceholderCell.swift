//
//  TimelineTimeEntryPlaceholderCell.swift
//  TogglDesktop
//
//  Created by Andrew Nester on 23.06.2020.
//  Copyright © 2020 Alari. All rights reserved.
//

import Cocoa

class TimelineTimeEntryPlaceholderCell: TimelineBaseCell {
    @IBOutlet weak var placeholderLabel: NSTextField!

    override func viewDidLoad() {
        super.viewDidLoad()

        placeholderLabel.stringValue = NSLocalizedString("New entry",
                                                         comment: "Placeholder text to show when dragg-creating new Time Entry")
    }

    override func viewWillAppear() {
        super.viewDidAppear()
        renderColor(with: TimeEntryViewItem.defaultProjectColor())
    }
}
