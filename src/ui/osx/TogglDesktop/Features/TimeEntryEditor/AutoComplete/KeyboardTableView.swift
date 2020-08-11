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
        keyDown(with: event)
        return true
    }

    override func keyDown(with event: NSEvent) {
        let key = TogglDesktop.Key(rawValue: Int(event.keyCode))
        switch key {
        case .upArrow, .downArrow:
            // pass the key to table view
            super.keyDown(with: event)
        default:
            break
        }

        // handle if need
        guard let scopeKey = Key(rawValue: event.keyCode) else { return }
        _ = keyDidDownOnPress?(scopeKey)
    }

    override func mouseDown(with event: NSEvent) {
        super.mouseDown(with: event)

        let point = convert(event.locationInWindow, from: nil)
        let rowIndex = row(at: point)
        clickedOnRow?(rowIndex)
    }

    override func acceptsFirstMouse(for event: NSEvent?) -> Bool {
        return true
    }
}
