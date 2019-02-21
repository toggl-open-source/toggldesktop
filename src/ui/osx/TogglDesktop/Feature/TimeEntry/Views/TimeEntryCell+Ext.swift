//
//  TimeEntryCell+Ext.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 2/21/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

extension TimeEntryCell {

    @objc enum ShadowMode: Int {
        case top
        case middle
        case bottom
        case all
    }

    @objc func drawShadow(with mode: ShadowMode) {

        view.wantsLayer = true
        view.layer?.masksToBounds = false
        view.shadow = NSShadow()
        view.layer?.shadowColor = NSColor.init(white: 0, alpha: 0.1).cgColor
        view.layer?.shadowOpacity = 1.0
        view.layer?.shadowOffset = CGSize(width: 0, height: -2)
        view.layer?.shadowRadius = 8
    }
}
