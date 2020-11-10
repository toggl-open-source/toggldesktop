//
//  Colors.swift
//  TogglDesktop
//
//  Created by Andrew Nester on 11.11.2020.
//  Copyright © 2020 Toggl. All rights reserved.
//

import Foundation

enum Color: String, CaseIterable {
    case green = "green-color"
    case errorTitle = "error-title-color"
    case greyText = "grey-text-color"
    case lighterGrey = "lighter-grey-color"
    case textFieldHoverBackground = "text-field-hover-background"
    case timelineBackgroundColor = "timeline-background-color"
    case collectionViewBackgroundColor = "collectionview-background-color"
    case blackText = "black-text-color"

    var color: NSColor {
        if #available(OSX 10.13, *) {
            return NSColor(named: rawValue)!
        } else {
            return legacyColor
        }
    }

    fileprivate var legacyColor: NSColor {
        switch self {
        case .green:
            return ConvertHexColor.hexCode(toNSColor: "#DF68D0")
        case .errorTitle:
            return ConvertHexColor.hexCode(toNSColor: "#FF3B30")
        case .greyText:
            return ConvertHexColor.hexCode(toNSColor: "#564360")
        case .lighterGrey:
            return ConvertHexColor.hexCode(toNSColor: "#ABA0AF")
        case .textFieldHoverBackground:
            return NSColor(red: 0, green: 0, blue: 0, alpha: 0.05)
        case .timelineBackgroundColor:
            return NSColor.clear
        case .collectionViewBackgroundColor:
            return ConvertHexColor.hexCode(toNSColor: "#FEF9F8")
        case .blackText:
            return ConvertHexColor.hexCode(toNSColor: "#2C1338")
        }
    }
}

@objc
extension NSColor {
    static var togglGreen: NSColor { Color.green.color }
    static var collectionViewBackgroundColor: NSColor { Color.collectionViewBackgroundColor.color }
}

@objc enum ColorObjc: Int {
    case black
    case white
}
