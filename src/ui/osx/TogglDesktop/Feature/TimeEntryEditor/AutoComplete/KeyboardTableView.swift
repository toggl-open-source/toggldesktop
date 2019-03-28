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

    enum Key: UInt16 {
        case returnKey                  = 0x24
        case enter                      = 0x4C
        case tab                       = 0x30
    }

    var keyUpOnPress: KeyUpClosure?

    override func keyDown(with event: NSEvent) {
        super.keyDown(with: event)
        guard let key = Key(rawValue: event.keyCode) else { return }
        keyUpOnPress?(key)
    }
}
