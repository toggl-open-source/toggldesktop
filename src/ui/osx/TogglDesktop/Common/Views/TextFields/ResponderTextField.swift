//
//  ResponderTextField.swift
//  TogglDesktop
//
//  Created by Andrew Nester on 10.08.2020.
//  Copyright © 2020 Toggl. All rights reserved.
//

import Foundation

protocol TextFieldResponderDelegate: AnyObject {
    func didBecomeFirstResponder(_ sender: NSTextField)
    func didResignFirstResponder(_ sender: NSTextField)
}

class ResponderTextField: NSTextField {
    weak var responderDelegate: TextFieldResponderDelegate?

    override func becomeFirstResponder() -> Bool {
        defer {
            responderDelegate?.didBecomeFirstResponder(self)
        }
        return super.becomeFirstResponder()
    }

    override func resignFirstResponder() -> Bool {
        defer {
            responderDelegate?.didResignFirstResponder(self)
        }
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
