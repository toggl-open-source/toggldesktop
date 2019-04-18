//
//  NSView+Apperance.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 3/12/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

extension NSView {
    
    @objc var isDarkMode: Bool {
        if #available(OSX 10.14, *) {
            if effectiveAppearance.name == .darkAqua {
                return true
            }
        }
        return false
    }
}

/// Check or get notified about macOS Dark Mode status
public final class DarkMode {
    private static let notificationName = NSNotification.Name("AppleInterfaceThemeChangedNotification")

    static var onChange: ((Bool) -> Void)? {
        didSet {
            if onChange == nil {
                DistributedNotificationCenter.default().removeObserver(self, name: notificationName, object: nil)
            } else {
                DistributedNotificationCenter.default().addObserver(self, selector: #selector(selectorHandler), name: notificationName, object: nil)
            }
        }
    }

    static var isEnabled: Bool {
        return UserDefaults.standard.string(forKey: "AppleInterfaceStyle") == "Dark"
    }

    static var blackColor: NSColor {
        return isEnabled ? .white : .black
    }

    @objc
    private static func selectorHandler() {
        onChange?(isEnabled)
    }
}
