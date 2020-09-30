//
//  AutocompleteSourceViewType.swift
//  TogglDesktop
//
//  Created by Andrew Nester on 30.09.2020.
//  Copyright © 2020 Toggl. All rights reserved.
//

import Foundation

extension TimerViewController {

    enum AutocompleteSourceViewType {
        case button(NSView)
        case textField(NSView)

        var sourceView: NSView {
            switch self {
            case .button(let view):
                return view
            case .textField(let field):
                return field
            }
        }

        var makeWindowKey: Bool {
            switch self {
            case .button:
                return true
            case .textField:
                return false
            }
        }

        var offset: NSPoint {
            switch self {
            case .button:
                return NSPoint(x: 0, y: 2)
            case .textField:
                return NSPoint(x: 8, y: 6)
            }
        }
    }
}
