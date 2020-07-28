//
//  NSColor+Theme.swift
//  TogglDesktop
//
//  Created by Andrew Nester on 28.07.2020.
//  Copyright © 2020 Toggl. All rights reserved.
//

import Foundation

// Helper methods to get color when OS version is not high enough to use Assets
@objc
extension NSColor {

    static var togglGreen: NSColor {
        if #available(OSX 10.13, *) {
            return NSColor(named: NSColor.Name("green-color"))!
        } else {
            return ConvertHexColor.hexCode(toNSColor: "#28cd41")
        }
    }

    static var togglErrorTitle: NSColor {
        if #available(OSX 10.13, *) {
            return NSColor(named: NSColor.Name("error-title-color"))!
        } else {
            return ConvertHexColor.hexCode(toNSColor: "#FF3B30")
        }
    }

    static var togglGreyText: NSColor {
        if #available(OSX 10.13, *) {
            return NSColor(named: NSColor.Name("grey-text-color"))!
        } else {
            return ConvertHexColor.hexCode(toNSColor: "#555555")
        }
    }

    static var togglLighterGrey: NSColor {
        if #available(OSX 10.13, *) {
            return NSColor(named: NSColor.Name("lighter-grey-color"))!
        } else {
            return ConvertHexColor.hexCode(toNSColor: "#acacac")
        }
    }
}
