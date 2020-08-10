//
//  ResponderTextField.swift
//  TogglDesktop
//
//  Created by Andrew Nester on 10.08.2020.
//  Copyright © 2020 Toggl. All rights reserved.
//

import Foundation

class ResponderTextField: NSTextField {
    weak var responderDelegate: TextFieldResponderDelegate?

    override func becomeFirstResponder() -> Bool {
        responderDelegate?.didBecomeFirstResponder(self)
        return super.becomeFirstResponder()
    }

    override func resignFirstResponder() -> Bool {
        responderDelegate?.didResignFirstResponder(self)
        return super.resignFirstResponder()
    }

    override func mouseDown(with event: NSEvent) {
        super.mouseDown(with: event)
        responderDelegate?.didBecomeFirstResponder(self)
    }

    override func textDidBeginEditing(_ notification: Notification) {
        super.textDidBeginEditing(notification)
        responderDelegate?.didBecomeFirstResponder(self)
    }

    override func textDidEndEditing(_ notification: Notification) {
        super.textDidEndEditing(notification)
        responderDelegate?.didResignFirstResponder(self)
    }
}
