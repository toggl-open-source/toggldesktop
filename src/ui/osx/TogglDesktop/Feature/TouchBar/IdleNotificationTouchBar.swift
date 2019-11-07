//
//  IdleNotificationTouchBar.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 11/1/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

@available(OSX 10.12.2, *)
final class IdleNotificationTouchBar: NSObject {

    // MARK: Variable

    private lazy var discardButton: NSButton = {
        let btn = NSButton(title: "Discard idle time", target: self, action: #selector(self.btnOnTap(_:)))
        btn.setButtonType(.momentaryPushIn)
        btn.bezelColor = NSColor.systemGreen
        return btn
    }()

    private lazy var discardAndContinueButton: NSButton = {
        let btn = NSButton(title: "Discard idle and continue", target: self, action: #selector(self.btnOnTap(_:)))
        btn.setButtonType(.momentaryPushIn)
        return btn
    }()

    private lazy var keepIdleButton: NSButton = {
        let btn = NSButton(title: "Keep idle time", target: self, action: #selector(self.btnOnTap(_:)))
        btn.setButtonType(.momentaryPushIn)
        return btn
    }()

    private lazy var addIdleTimeButton: NSButton = {
        let btn = NSButton(title: "Add idle time as new time entry", target: self, action: #selector(self.btnOnTap(_:)))
        btn.setButtonType(.momentaryPushIn)
        return btn
    }()

    // MARK: Public

    func makeTouchBar() -> NSTouchBar {
        let touchBar = NSTouchBar()
        touchBar.delegate = self
        touchBar.customizationIdentifier = .idleNotificationTouchBar
        touchBar.defaultItemIdentifiers = [.flexibleSpace,
                                           .discardIdleItem,
                                           .discardIdleAndContinueItem,
                                           .keepIdleTimeItem,
                                           .addIdleTimeItem,
                                           .flexibleSpace]
        return touchBar
    }

    @objc private func btnOnTap(_ sender: NSButton) {

    }
}

// MARK: Private

@available(OSX 10.12.2, *)
extension IdleNotificationTouchBar: NSTouchBarDelegate {

    func touchBar(_ touchBar: NSTouchBar, makeItemForIdentifier identifier: NSTouchBarItem.Identifier) -> NSTouchBarItem? {
        switch identifier {
        case NSTouchBarItem.Identifier.discardIdleItem:
            let item = NSCustomTouchBarItem(identifier: identifier)
            item.visibilityPriority = .high
            item.view = discardButton
            return item
        case NSTouchBarItem.Identifier.discardIdleAndContinueItem:
            let item = NSCustomTouchBarItem(identifier: identifier)
            item.visibilityPriority = .high
            item.view = discardAndContinueButton
            return item
        case NSTouchBarItem.Identifier.keepIdleTimeItem:
            let item = NSCustomTouchBarItem(identifier: identifier)
            item.visibilityPriority = .high
            item.view = keepIdleButton
            return item
        case NSTouchBarItem.Identifier.addIdleTimeItem:
            let item = NSCustomTouchBarItem(identifier: identifier)
            item.visibilityPriority = .high
            item.view = addIdleTimeButton
            return item
        default:
            return nil
        }
    }
}
