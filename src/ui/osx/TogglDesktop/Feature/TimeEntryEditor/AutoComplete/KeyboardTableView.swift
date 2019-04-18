//
//  KeyboardTableView.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 3/28/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

final class KeyboardTableView: NSTableView {

    typealias KeyClosure = (Key) -> Bool
    typealias ClickedOnRowClosure = (Int) -> Void

    enum Key: UInt16 {
        case returnKey                  = 0x24
        case enter                      = 0x4C
        case tab                        = 0x30
        case downArrow                  = 125
    }

    // MARK: Variables

    var keyDidDownOnPress: KeyClosure?
    var clickedOnRow: ClickedOnRowClosure?

    // MARK: Public

    func handleKeyboardEvent(_ event: NSEvent) -> Bool {

        // Pass the key to table view
        keyDown(with: event)

        // Handle if need
        guard let key = Key(rawValue: event.keyCode) else { return false }
        return keyDidDownOnPress?(key) ?? false
    }

    override func mouseDown(with event: NSEvent) {
        super.mouseDown(with: event)

        let point = convert(event.locationInWindow, from: nil)
        let rowIndex = row(at: point)
        clickedOnRow?(rowIndex)
    }
}
