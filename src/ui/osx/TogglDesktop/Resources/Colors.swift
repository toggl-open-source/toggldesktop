//
//  Colors.swift
//  TogglDesktop
//
//  Created by Andrew Nester on 11.11.2020.
//  Copyright © 2020 Toggl. All rights reserved.
//

import Foundation

// swiftlint:disable object_literal
enum Color: String, CaseIterable {
    case green = "green-color"
    case errorTitle = "error-title-color"
    case errorBackground = "error-background-color"
    case greyText = "grey-text-color"
    case lighterGrey = "lighter-grey-color"
    case textFieldHoverBackground = "text-field-hover-background"
    case timelineBackgroundColor = "timeline-background-color"
    case timelineDatePickerBackground = "timeline-date-picker-background-color"
    case timelineBackgroundHighlight = "timeline-background-highlight-color"
    case timelineDivider = "timeline-divider-color"
    case collectionViewBackgroundColor = "collectionview-background-color"
    case blackText = "black-text-color"
    case borderDefault = "border-default"
    case whiteBackgroundColor = "white-background-color"
    case projectColorButtonBorder = "color-project-btn-border-color"
    case shadow = "shadow"
    case onboardingBackground = "onboarding-background-color"
    case loginButtonBackground = "login-button-background"
    case autoCompleteCellHover = "auto-complete-cell-hover"
    case tagSelectionBackground = "tag-selection-background-color"
    case uploadBorder = "upload-border-color"
    case tabViewText = "tab-view-text-color"

    var color: NSColor {
        if #available(OSX 10.13, *) {
            return NSColor(named: rawValue)!
        } else {
            return legacyColor
        }
    }

    var cgColor: CGColor {
        return color.cgColor
    }

    fileprivate var legacyColor: NSColor {
        switch self {
        case .green:
            return ConvertHexColor.hexCode(toNSColor: "#DF68D0")
        case .errorTitle:
            return ConvertHexColor.hexCode(toNSColor: "#FF3B30")
        case .errorBackground:
            return NSColor.white
        case .greyText:
            return ConvertHexColor.hexCode(toNSColor: "#564360")
        case .lighterGrey:
            return ConvertHexColor.hexCode(toNSColor: "#ABA0AF")
        case .textFieldHoverBackground:
            return NSColor(red: 0, green: 0, blue: 0, alpha: 0.05)
        case .timelineBackgroundColor:
            return NSColor.white
        case .timelineDatePickerBackground:
            return NSColor.white
        case .timelineBackgroundHighlight:
            return ConvertHexColor.hexCode(toNSColor: "#f7f7f7")
        case .timelineDivider:
            return ConvertHexColor.hexCode(toNSColor: "#e8e8e8")
        case .collectionViewBackgroundColor:
            return ConvertHexColor.hexCode(toNSColor: "#FEF9F8")
        case .blackText:
            return ConvertHexColor.hexCode(toNSColor: "#2C1338")
        case .borderDefault,
             .projectColorButtonBorder,
             .shadow:
            return NSColor.black.withAlphaComponent(0.1)
        case .whiteBackgroundColor:
            return NSColor.white
        case .onboardingBackground:
            return NSColor.black.withAlphaComponent(0.5)
        case .loginButtonBackground:
            return ConvertHexColor.hexCode(toNSColor: "#DF68D0")
        case .autoCompleteCellHover:
            return NSColor(white: 0.7, alpha: 0.2)
        case .tagSelectionBackground:
            return ConvertHexColor.hexCode(toNSColor: "#e5f9e8")
        case .uploadBorder:
            return ConvertHexColor.hexCode(toNSColor: "#ACACAC")
        case .tabViewText:
            return ConvertHexColor.hexCode(toNSColor: "#ACACAC")
        }
    }
}

@objc
extension NSColor {
    static var togglGreen: NSColor { Color.green.color }
    static var collectionViewBackgroundColor: NSColor { Color.collectionViewBackgroundColor.color }
    static var shadow: NSColor { Color.shadow.color }
}

@objc enum ColorObjc: Int {
    case black
    case white
}
