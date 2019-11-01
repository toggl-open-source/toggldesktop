//
//  AlertTouchBar.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 11/1/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

@available(OSX 10.12.2, *)
final class AlertTouchBar: NSObject {

    // MARK: Variable

    private lazy var cancelButton: NSButton = {
        let btn = NSButton(title: "Cancel", target: self, action: #selector(self.cancelBtnOnTap(_:)))
        return btn
    }()

    private lazy var deleteButton: NSButton = {
        let btn = NSButton(title: "Delete", target: self, action: #selector(self.deleteBtnOnTap(_:)))
        return btn
    }()

    // MARK: Public

    func makeTouchBar() -> NSTouchBar {
        let touchBar = NSTouchBar()
        touchBar.delegate = self
        touchBar.customizationIdentifier = .mainTouchBar
        touchBar.defaultItemIdentifiers = [.flexibleSpace, .cancelItem, .fixedSpaceSmall, .deleteItem, .flexibleSpace]
        return touchBar
    }

    @objc private func cancelBtnOnTap(_ sender: NSButton) {

    }

    @objc private func deleteBtnOnTap(_ sender: NSButton) {

    }
}

// MARK: Private

@available(OSX 10.12.2, *)
extension AlertTouchBar: NSTouchBarDelegate {

    func touchBar(_ touchBar: NSTouchBar, makeItemForIdentifier identifier: NSTouchBarItem.Identifier) -> NSTouchBarItem? {
        switch identifier {
        case NSTouchBarItem.Identifier.cancelItem:
            let item = NSCustomTouchBarItem(identifier: identifier)
            item.visibilityPriority = .high
            item.view = cancelButton
            return item
        case NSTouchBarItem.Identifier.deleteItem:
            let item = NSCustomTouchBarItem(identifier: identifier)
            item.visibilityPriority = .high
            item.view = deleteButton
            return item
        default:
            return nil
        }
    }
}
