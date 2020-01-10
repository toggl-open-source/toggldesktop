//
//  KeySender.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 1/10/20.
//  Copyright © 2020 Alari. All rights reserved.
//
// Based on https://github.com/brianmichel/ESCapey/blob/master/ESCapey-macOS/ESCapey-macOS/KeySender.swift

import Foundation

protocol KeySender {
    var keyCode: CGKeyCode { get }
    func send()
}

extension KeySender {
    func send() {
        let downEvent = CGEvent(keyboardEventSource: nil, virtualKey: keyCode, keyDown: true)
        let upEvent = CGEvent(keyboardEventSource: nil, virtualKey: keyCode, keyDown: false)

        downEvent?.post(tap: .cghidEventTap)
        upEvent?.post(tap: .cghidEventTap)
    }
}

struct ESCKeySender: KeySender {
    let keyCode: CGKeyCode = 53
}
