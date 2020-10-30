//
//  ResponderTextField.swift
//  TogglDesktop
//
//  Created by Andrew Nester on 10.08.2020.
//  Copyright © 2020 Toggl. All rights reserved.
//

import Foundation

class ResponderTextField: NSTextField, ResponderObservable {

    var observations = (
        becomeResponder: [UUID: () -> Void](),
        resignResponder: [UUID: () -> Void]()
    )

    override func becomeFirstResponder() -> Bool {
        defer {
            observations.becomeResponder.values.forEach { $0() }
        }
        return super.becomeFirstResponder()
    }

    override func resignFirstResponder() -> Bool {
        defer {
            observations.resignResponder.values.forEach { $0() }
        }
        return super.resignFirstResponder()
    }

    override func mouseDown(with event: NSEvent) {
        super.mouseDown(with: event)
        observations.becomeResponder.values.forEach { $0() }
    }

    override func textDidBeginEditing(_ notification: Notification) {
        super.textDidBeginEditing(notification)
        observations.becomeResponder.values.forEach { $0() }
    }

    override func textDidEndEditing(_ notification: Notification) {
        super.textDidEndEditing(notification)
        observations.resignResponder.values.forEach { $0() }
    }
}
