//
//  KeyboardTableView.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 3/28/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

final class KeyboardTableView: NSTableView {

    typealias KeyUpClosure = (Key) -> Void
    typealias ClickedOnRowClosure = (Int) -> Void

    enum Key: UInt16 {
        case returnKey                  = 0x24
        case enter                      = 0x4C
        case tab                       = 0x30
        case downArrow = 125
    }

    // MARK: Variables

    var keyWillDownOnPress: (() -> Void)?
    var keyDidDownOnPress: KeyUpClosure?
    var clickedOnRow: ClickedOnRowClosure?

    // MARK: Public

    override func keyDown(with event: NSEvent) {
        keyWillDownOnPress?()
        super.keyDown(with: event)
        guard let key = Key(rawValue: event.keyCode) else { return }
        keyDidDownOnPress?(key)
    }

    override func mouseDown(with event: NSEvent) {
        super.mouseDown(with: event)

        let point = convert(event.locationInWindow, from: nil)
        let rowIndex = row(at: point)
        clickedOnRow?(rowIndex)
    }
}
