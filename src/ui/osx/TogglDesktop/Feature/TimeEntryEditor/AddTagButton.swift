//
//  AddTagButton.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 6/13/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa
import Carbon.HIToolbox

protocol AddTagButtonDelegate: class {
    func shouldOpenTagAutoComplete(with text: String)
}

final class AddTagButton: NSTextField {

    // MARK: Variables

    weak var keyboardDelegate: AddTagButtonDelegate?
    private let ignoreKeys = [kVK_Tab,kVK_Space] // Tab and space

    var cursor: NSCursor? {
        didSet {
            resetCursorRects()
        }
    }

    override func awakeFromNib() {
        super.awakeFromNib()
        cursor = .pointingHand
    }

    // MARK: Override

    override func resetCursorRects() {
        if let cursor = cursor {
            addCursorRect(bounds, cursor: cursor)
        } else {
            super.resetCursorRects()
        }
    }

    override func keyDown(with event: NSEvent) {
        super.keyDown(with: event)

        // Open the auto complete if the key isn't ignore key
        guard let characters = event.characters, !ignoreKeys.contains(Int(event.keyCode)) else { return }

        // Replace "enter" with empty string if need
        let text = characters == "\r" ? "" : characters

        // Notify
        keyboardDelegate?.shouldOpenTagAutoComplete(with: text)
    }

    override func mouseDown(with event: NSEvent) {
        super.mouseDown(with: event)
        keyboardDelegate?.shouldOpenTagAutoComplete(with: "")
    }
}
