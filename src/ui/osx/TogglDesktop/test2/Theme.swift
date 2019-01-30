//
//  Theme.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 1/15/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

enum Theme {

    case dark
    case light

    static func current() -> Theme {
        let mode = UserDefaults.standard.string(forKey: "AppleInterfaceStyle")
        return mode == "Dark" ? .dark : .light
    }
}

@objc class ThemeUtils: NSObject {

    @objc class func isDarkTheme() -> Bool {
        return Theme.current() == .dark
    }
}
