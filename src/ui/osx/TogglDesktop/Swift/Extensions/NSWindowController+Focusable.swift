//
//  NSWindowController+Focusable.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 8/16/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

extension NSWindowController {

    @objc func showWindowAndFocus() {
        showWindow(self)
        NSApp.activate(ignoringOtherApps: true)
    }
}
