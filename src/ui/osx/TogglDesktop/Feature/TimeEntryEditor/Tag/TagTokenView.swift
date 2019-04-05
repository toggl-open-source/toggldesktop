//
//  TagTokenView.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 4/5/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

final class TagTokenView: NSView {

    // MARK: OUTLET

    @IBOutlet weak var boxContainerView: NSBox!
    @IBOutlet weak var titleLabel: NSTextField!
    @IBOutlet weak var closeButton: CursorButton!

    // MARK: Views

    override func awakeFromNib() {
        super.awakeFromNib()

        initCommon()
    }

    func render(_ tag: Tag) {
        titleLabel.stringValue = tag.name
    }
}

// MARK: Private

extension TagTokenView {

    fileprivate func initCommon() {
        closeButton.isHidden = true
        closeButton.cursor = .pointingHand
    }
}
