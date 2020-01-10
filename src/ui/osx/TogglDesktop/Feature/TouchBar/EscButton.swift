//
//  EscButton.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 1/10/20.
//  Copyright © 2020 Alari. All rights reserved.
//

import Foundation

final class EscButton: NSButton {

    private let key = ESCKeySender()

    override init(frame frameRect: NSRect) {
        super.init(frame: frameRect)
        initCommon()
    }

    required init?(coder: NSCoder) {
        super.init(coder: coder)
        initCommon()
    }

    override public var intrinsicContentSize: NSSize {
        var size = super.intrinsicContentSize
        size.width = min(size.width, 64)
        return size
    }

    @objc private func tap() {
        key.send()
    }

    private func initCommon() {
        target = self
        action = #selector(self.tap)
    }
}
