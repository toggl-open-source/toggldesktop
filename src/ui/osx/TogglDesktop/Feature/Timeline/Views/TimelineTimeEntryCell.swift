//
//  TimelineTimeEntryCell.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 6/21/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

final class TimelineTimeEntryCell: NSCollectionViewItem {

    // MARK: OUTLET

    @IBOutlet weak var backgroundView: NSBox!

    // MARK: View

    override func viewDidLoad() {
        super.viewDidLoad()
        // Do view setup here.
    }

    // MARK: Public

    func config(for timeEntry: TimelineTimeEntry) {
        backgroundView.fillColor = timeEntry.color
        backgroundView.cornerRadius = timeEntry.isSmall ? 1.0 : 10.0
    }

    @IBAction func timeEntryOnTap(_ sender: Any) {
    }
}
