//
//  NoInteractionView.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 5/28/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

@objc protocol NoInteractionViewDelegate: class {

    func noInteractionViewMouseDidDown(_ sender: NoInteractionView)
}

@objcMembers final class NoInteractionView: NSView {

    // MARK: Variables

    weak var delegate: NoInteractionViewDelegate?

    // MARK: Override

    override func mouseDown(with event: NSEvent) {
        // Don't call the super.mouseDown()
        // to ignore the action on the Time Entry Collection View
        // However, we close the AutoCompletel View
        delegate?.noInteractionViewMouseDidDown(self)
    }
}
