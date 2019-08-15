//
//  AddTagButton.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 6/13/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

protocol AddTagButtonDelegate: class {
    func shouldOpenTagAutoComplete(with text: String)
}

final class AddTagButton: NSTextField {

    // MARK: Variables

    weak var keyboardDelegate: AddTagButtonDelegate?
    private var cursor: NSCursor? {
        didSet {
            resetCursorRects()
        }
    }

    override func awakeFromNib() {
        super.awakeFromNib()
        delegate = self
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

    override func mouseDown(with event: NSEvent) {
        super.mouseDown(with: event)
        keyboardDelegate?.shouldOpenTagAutoComplete(with: "")
    }
}

// MARK: NSTextFieldDelegate

extension AddTagButton: NSTextFieldDelegate {

    func controlTextDidChange(_ obj: Notification) {

        // Send the current key to auto complete
        keyboardDelegate?.shouldOpenTagAutoComplete(with: stringValue)

        // Reset to empty
        stringValue = ""
    }

    func control(_ control: NSControl, textView: NSTextView, doCommandBy commandSelector: Selector) -> Bool {
        if (commandSelector == #selector(NSResponder.insertNewline(_:))) {
            keyboardDelegate?.shouldOpenTagAutoComplete(with: "")
            return true
        }
        return false
    }
}
