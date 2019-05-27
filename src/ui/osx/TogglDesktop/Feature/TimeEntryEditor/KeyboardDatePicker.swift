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

    var escapeKeyOnAction: (() -> Void)?

    override func keyDown(with event: NSEvent) {
        super.keyDown(with: event)

        if event.keyCode == UInt16(kVK_Escape) {
            escapeKeyOnAction?()
        }
    }
}
