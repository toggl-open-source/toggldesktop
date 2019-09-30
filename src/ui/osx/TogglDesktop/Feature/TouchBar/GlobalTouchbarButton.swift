//
//  GlobalTouchbarButton.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 9/30/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

@available(OSX 10.12.2, *)
@objcMembers
final class GlobalTouchbarButton: NSCustomTouchBarItem {

    // MARK: Variables

    static let ID = NSTouchBarItem.Identifier("toggl.touchbar")
    private let button: NSButton

    // MARK: Init

    override init(identifier: NSTouchBarItem.Identifier) {
        let image = NSImage(named: "off")!
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
        return GlobalTouchbarButton(identifier: GlobalTouchbarButton.ID)
    }

    // MARK: Public

    func update(_ iconImage: NSImage) {
        // This forces the icon to redraw
        button.image = iconImage
        button.imagePosition = .imageOnly
    }
}

// MARK: Private

@available(OSX 10.12.2, *)
extension GlobalTouchbarButton {

    @objc fileprivate func buttonOnTap() {

    }
}
