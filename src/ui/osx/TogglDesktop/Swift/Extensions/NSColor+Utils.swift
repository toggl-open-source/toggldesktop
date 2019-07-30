//
//  NSColor+Utils.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 1/16/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

extension NSColor {

    @objc class var safeUnemphasizedSelectedContentBackgroundColor: NSColor {
        if #available(OSX 10.14, *) {
            return NSColor.unemphasizedSelectedContentBackgroundColor
        } else {
            return NSColor(white: 0.21, alpha: 1.0)
        }
    }
}
