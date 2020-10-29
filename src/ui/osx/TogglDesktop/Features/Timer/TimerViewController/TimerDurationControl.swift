//
//  TimerDurationControl.swift
//  TogglDesktop
//
//  Created by Andrew Nester on 29.10.2020.
//  Copyright © 2020 Toggl. All rights reserved.
//

import Foundation

class TimerDurationControl: NSView {

    var onDurationTextChange: ((String) -> Void)?

    var stringValue: String {
        get { durationTextField.stringValue }
        set { durationTextField.stringValue = newValue }
    }

    var isEditing: Bool {
        durationTextField.currentEditor() != nil
    }

    override var canBecomeKeyView: Bool { true }
    override var acceptsFirstResponder: Bool { true }
    override var needsPanelToBecomeKey: Bool { true }

    enum Action: Equatable {
        case enterPress
    }

    /// Called when control requests an action to be performed.
    /// Closure must return `true` if action was handled.
    var onPerformAction: (Action) -> Bool = { _ in return false }

    // MARK: IBOutlet

    @IBOutlet private weak var durationTextField: ResponderTextField!
    @IBOutlet private weak var backgroundBox: TimerContainerBox!

    override func awakeFromNib() {
        super.awakeFromNib()
        setup()
    }

    private func setup() {
        durationTextField.responderDelegate = backgroundBox
        durationTextField.delegate = self
    }

    override func becomeFirstResponder() -> Bool {
        durationTextField.nextKeyView = nextKeyView
        return durationTextField.becomeFirstResponder()
    }
}

extension TimerDurationControl: NSTextFieldDelegate {
    func controlTextDidEndEditing(_ obj: Notification) {
        onDurationTextChange?(durationTextField.stringValue)
    }

    func control(_ control: NSControl, textView: NSTextView, doCommandBy commandSelector: Selector) -> Bool {
        if commandSelector == #selector(insertNewline(_:)) {
            window?.makeFirstResponder(nil)
            return onPerformAction(.enterPress)
        }

        if commandSelector == #selector(insertBacktab(_:)) {
            window?.selectKeyView(preceding: self)
            return true
        }

        return false
    }
}
