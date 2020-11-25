//
//  KeyButton.swift
//  TogglDesktop
//
//  Created by Andrew Nester on 25.11.2020.
//  Copyright © 2020 Toggl. All rights reserved.
//

import Foundation

class KeyButton: NSButton {

    var didPressKey: ((Key, NSEvent.ModifierFlags) -> Void)?

    var activateOnEnterKey = false

    private var _canBecomeKeyView = false
    override var canBecomeKeyView: Bool { _canBecomeKeyView }

    func setCanBecomeKeyView(_ canBecome: Bool) {
        _canBecomeKeyView = canBecome
    }

    override func keyDown(with event: NSEvent) {
        let key = Key(rawValue: Int(event.keyCode))

        if let key = key, key == .enter && activateOnEnterKey {
            sendAction(action, to: target)
        } else {
            super.keyDown(with: event)
        }

        if let key = key {
            didPressKey?(key, event.modifierFlags)
        }
    }
}
