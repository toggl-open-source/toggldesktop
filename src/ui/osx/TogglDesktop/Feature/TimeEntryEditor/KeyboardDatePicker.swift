//
//  KeyboardDatePicker.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 5/27/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation
import Carbon.HIToolbox

final class KeyboardDatePicker: NSDatePicker {

    enum Key {
        case escape
        case space

        init?(rawValue: Int) {
            switch rawValue {
            case kVK_Escape:
                self = .escape
            case kVK_Space:
                self = .space
            default:
                return nil
            }
        }
    }

    // MARK: Variable

    var keyOnAction: ((Key) -> Void)?

    override func keyDown(with event: NSEvent) {
        super.keyDown(with: event)

        guard let key = Key(rawValue: Int(event.keyCode)) else { return }
        keyOnAction?(key)
    }
}
