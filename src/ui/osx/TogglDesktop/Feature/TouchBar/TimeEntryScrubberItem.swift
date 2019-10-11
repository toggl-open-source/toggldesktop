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

    // MARK: OUTLET

    private lazy var titleBtn = NSButton(title: "", target: nil, action: nil)

    // MARK: View Cycle

    override func awakeFromNib() {
        super.awakeFromNib()
        if titleBtn.superview == nil {
            titleBtn.setButtonType(NSButton.ButtonType.momentaryChange)
            titleBtn.translatesAutoresizingMaskIntoConstraints = false
            addSubview(titleBtn)
            titleBtn.centerXAnchor.constraint(equalTo: self.centerXAnchor).isActive = true
            titleBtn.centerYAnchor.constraint(equalTo: self.centerYAnchor).isActive = true
            titleBtn.leadingAnchor.constraint(equalTo: self.leadingAnchor, constant: 0).isActive = true
            titleBtn.trailingAnchor.constraint(equalTo: self.trailingAnchor, constant: 0).isActive = true
        }
    }

    // MARK: Public

    func config(_ item: TimeEntryViewItem) {
        titleBtn.title = item.touchBarTitle
        titleBtn.setTextColor(NSColor.white)
    }
}
