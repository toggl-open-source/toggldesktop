//
//  TagCellView.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 4/5/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

final class TagCellView: HoverTableCellView {

    static let cellHeight: CGFloat = 34.0

    private lazy var backgroundColor: NSColor = {
        if #available(OSX 10.13, *) {
            return NSColor(named: NSColor.Name("tag-selection-background-color"))!
        } else {
            return ConvertHexColor.hexCode(toNSColor: "#e5f9e8")
        }
    }()
    // MARK: OUTLET

    @IBOutlet weak var checkButton: NSButton!
    @IBOutlet weak var backgroundView: NSBox!

    // MARK: Public

    override func prepareForReuse() {
        super.prepareForReuse()
        checkButton.state = .off
    }

    func render(_ tag: Tag, isSelected: Bool) {
        checkButton.title = tag.name
        checkButton.state = isSelected ? .on : .off
        backgroundView.fillColor = isSelected ? backgroundColor : .clear
    }
}
