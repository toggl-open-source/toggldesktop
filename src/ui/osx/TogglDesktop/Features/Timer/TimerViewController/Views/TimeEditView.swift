//
//  TimeEditView.swift
//  TogglDesktop
//
//  Created by Andrew Nester on 30.10.2020.
//  Copyright © 2020 Toggl. All rights reserved.
//

import Cocoa

class TimeEditView: NSView {

    var onStartTextChange: ((String) -> Void)?

    var startStringValue: String {
        get {
            startTextField.stringValue
        }
        set {
            startTextField.stringValue = newValue
            isStartFieldChanged = false
        }
    }

    private var isStartFieldChanged = false

    @IBOutlet private weak var startTextField: NSTextField!

    override func becomeFirstResponder() -> Bool {
        startTextField.becomeFirstResponder()
    }
}

extension TimeEditView: NSTextFieldDelegate {
    func controlTextDidEndEditing(_ obj: Notification) {
        if obj.object as? NSTextField == startTextField, isStartFieldChanged {
            onStartTextChange?(startStringValue)
        }
    }

    func controlTextDidChange(_ obj: Notification) {
        isStartFieldChanged = true
    }

    func control(_ control: NSControl, textView: NSTextView, doCommandBy commandSelector: Selector) -> Bool {
        if commandSelector == #selector(insertTab(_:))
            || commandSelector == #selector(insertBacktab(_:))
            || commandSelector == #selector(cancelOperation(_:))
            || commandSelector == #selector(insertNewline(_:)) {
                window?.orderOut(nil)
        }
        return false
    }
}
