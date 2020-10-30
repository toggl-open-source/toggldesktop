//
//  TimeEditView.swift
//  TogglDesktop
//
//  Created by Andrew Nester on 30.10.2020.
//  Copyright © 2020 Toggl. All rights reserved.
//

import Cocoa

class TimeEditView: NSView {

    @IBOutlet private weak var startTextField: NSTextField!

    override func becomeFirstResponder() -> Bool {
        startTextField.becomeFirstResponder()
    }
}

extension TimeEditView: NSTextFieldDelegate {
    func control(_ control: NSControl, textView: NSTextView, doCommandBy commandSelector: Selector) -> Bool {
        if commandSelector == #selector(insertTab(_:))
            || commandSelector == #selector(insertBacktab(_:))
            || commandSelector == #selector(cancelOperation(_:)) {
                window?.orderOut(nil)
                return true
        }
        return false
    }
}
