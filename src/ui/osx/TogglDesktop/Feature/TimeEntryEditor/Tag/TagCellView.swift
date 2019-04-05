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

    // MARK: OUTLET

    @IBOutlet weak var checkButton: NSButton!

    // MARK: Public

    func render(_ tag: Tag) {
        checkButton.title = tag.name
    }
}
