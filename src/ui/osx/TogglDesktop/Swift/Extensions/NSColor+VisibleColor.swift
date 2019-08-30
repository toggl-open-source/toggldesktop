//
//  NSColor+VisibleColor.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 8/12/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

extension NSColor {

    @objc func visibleColor() -> NSColor {
        guard let window = NSApplication.shared.keyWindow else { return self }
        let darkMode = window.isDarkMode
        let bottomThreshold: CGFloat = 75.0 / 255.0
        let topThreshold: CGFloat = 180.0 / 255.0

        // Get correct RGB from current color space
        guard let component = getRGBComponents() else { return self }
        let red = component.red
        let green = component.green
        let blue = component.blue

        // If darkmode and color is black-based
        // or light mode and color is white-based
        if (darkMode && (red + blue + green) <= 3.0 * bottomThreshold) ||
            (!darkMode && (red + blue + green) >= 3.0 * topThreshold) {

            // Revert color to make it visible
            let red = 1.0 - redComponent
            let green = 1.0 - greenComponent
            let blue = 1.0 - blueComponent
            return NSColor(red: red, green: green, blue: blue, alpha: 1.0)
        }
        return self
    }

    func getRGBComponents() -> (red: CGFloat, green: CGFloat, blue: CGFloat)? {
        var red: CGFloat = 0
        var green: CGFloat = 0
        var blue: CGFloat = 0
        guard let rgbColor = self.usingColorSpace(NSColorSpace.deviceRGB) else { return nil }
        rgbColor.getRed(&red, green: &green, blue: &blue, alpha: nil)
        return (red, green, blue)
    }
}
