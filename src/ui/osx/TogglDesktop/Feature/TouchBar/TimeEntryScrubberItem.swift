//
//  TimeEntryScrubberItem.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 10/8/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

@available(OSX 10.12.2, *)
final class TimeEntryScrubberItem: NSScrubberItemView {

    lazy var titleBtn = NSButton(title: "", target: nil, action: nil)
    override func awakeFromNib() {
        super.awakeFromNib()

        if titleBtn.superview == nil {
            titleBtn.translatesAutoresizingMaskIntoConstraints = false
            addSubview(titleBtn)
            titleBtn.centerXAnchor.constraint(equalTo: self.centerXAnchor).isActive = true
            titleBtn.centerYAnchor.constraint(equalTo: self.centerYAnchor).isActive = true
        }
    }
    // MARK: Public

    func config(_ item: TimeEntryViewItem) {
        titleBtn.title = item.touchBarTitle
        titleBtn.setTextColor(NSColor.white)
    }
}
