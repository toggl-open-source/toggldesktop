//
//  Key.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 7/23/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation
import Carbon.HIToolbox

enum Key {
    case escape
    case space
    case enter
    case tab
    case backspace
    case delete

    init?(rawValue: Int) {
        switch rawValue {
        case kVK_Escape:
            self = .escape
        case kVK_Space:
            self = .space
        case kVK_Return:
            self = .enter
        case kVK_Tab:
            self = .tab
        case kBackspaceCharCode:
            self = .backspace
        case kVK_Delete:
            self = .delete
        default:
            return nil
        }
    }
}
