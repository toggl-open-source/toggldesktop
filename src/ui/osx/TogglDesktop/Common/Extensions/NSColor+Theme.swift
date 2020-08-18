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
            return ConvertHexColor.hexCode(toNSColor: "#DF68D0")
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
            return ConvertHexColor.hexCode(toNSColor: "#564360")
        }
    }

    static var togglLighterGrey: NSColor {
        if #available(OSX 10.13, *) {
            return NSColor(named: NSColor.Name("lighter-grey-color"))!
        } else {
            return ConvertHexColor.hexCode(toNSColor: "#ABA0AF")
        }
    }

    static var togglTextFieldHoverBackground: NSColor {
        if #available(OSX 10.13, *) {
            return NSColor(named: NSColor.Name("text-field-hover-background"))!
        } else {
            return NSColor(red: 0, green: 0, blue: 0, alpha: 0.05)
        }
    }

    static var timelineBackgroundColor: NSColor {
        if #available(OSX 10.13, *) {
            return NSColor(named: NSColor.Name("timeline-background-color"))!
        } else {
            return NSColor.clear
        }
    }

    static var collectionViewBackgroundColor: NSColor {
        if #available(OSX 10.13, *) {
            return NSColor(named: NSColor.Name("collectionview-background-color"))!
        } else {
            return ConvertHexColor.hexCode(toNSColor: "#FEF9F8")
        }
    }

    static var togglBlackText: NSColor {
        if #available(OSX 10.13, *) {
            return NSColor(named: NSColor.Name("black-text-color"))!
        } else {
            return ConvertHexColor.hexCode(toNSColor: "#2C1338")
        }
    }
}
