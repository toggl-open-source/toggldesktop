//
//  GlobalTouchbarButton.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 9/30/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

@available(OSX 10.12.2, *)
final class GlobalTouchbarButton: NSCustomTouchBarItem {

    private struct Constants {
        static let IDTouchBar = NSTouchBarItem.Identifier("toggl.touchbar")
    }

    // MARK: Variables

    private let button: NSButton

    // MARK: Init

    override init(identifier: NSTouchBarItem.Identifier) {
        let image = NSImage(named: "on")!
        image.isTemplate = true
        self.button = NSButton(image: image, target: nil, action: nil)
        self.button.imagePosition = .imageOnly
        super.init(identifier: identifier)
        self.button.target = self
        self.button.action = #selector(self.buttonOnTap)
        self.view = self.button
    }

    required init?(coder: NSCoder) {
        fatalError("Don't support coder")
    }

    class func makeDefault() -> GlobalTouchbarButton {
        let item = GlobalTouchbarButton(identifier: Constants.IDTouchBar)
        return item
    }

    // MARK: Public


}

// MARK: Private

@available(OSX 10.12.2, *)
extension GlobalTouchbarButton {

    @objc fileprivate func buttonOnTap() {

    }
}
