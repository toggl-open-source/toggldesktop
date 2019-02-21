//
//  NSView+Shadow.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 2/21/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

extension NSView {

    @objc enum ShadowMode: Int {
        case top
        case middle
        case bottom
        case all
    }

    @objc func drawShadow(with mode: ShadowMode) {

        wantsLayer = true
        layer?.masksToBounds = false
        shadow = NSShadow()
        layer?.shadowColor = NSColor.init(white: 0, alpha: 0.1).cgColor
        layer?.shadowOpacity = 1.0
        layer?.shadowOffset = CGSize(width: 0, height: -2)
        layer?.shadowRadius = 8
    }

    @objc func drawShadow(at index: Int, count: Int) {
        let mode: ShadowMode
        if index == 0 {
            mode = .top
        } else if index == (count - 1) {
            mode = .bottom
        } else {
            mode = .middle
        }

        drawShadow(with: mode)
    }
}
