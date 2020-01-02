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

        // Get correct RGB from current color space
        guard let component = getRGBComponents() else { return self }
        let red = component.red
        let green = component.green
        let blue = component.blue

        // If darkmode and color is black-based
        // or light mode and color is white-based
        let relativeLuminance = getRelativeLuminance(red: red, green: green, blue: blue)
        let contrastRatio = darkMode
            ? (relativeLuminance + 0.05)
                / (getRelativeLuminance(red: 30.0 / 255.0, green: 30.0 / 255.0, blue: 30.0 / 255.0) + 0.05)
            : (getRelativeLuminance(red: 1.0, green: 1.0, blue: 1.0) + 0.05)
                / (relativeLuminance + 0.05)
        
        if (contrastRatio < 1.5) {
            // Revert color to make it visible
            let red = 1.0 - red
            let green = 1.0 - green
            let blue = 1.0 - blue
            return NSColor(red: red, green: green, blue: blue, alpha: 1.0)
        }
        else if (contrastRatio < 3.0) {
            // Just make it lighter/darker
            let red = addContrast(component: red, darkMode: darkMode)
            let green = addContrast(component: green, darkMode: darkMode)
            let blue = addContrast(component: blue, darkMode: darkMode)
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

    func getRelativeLuminance(red: CGFloat, green: CGFloat, blue: CGFloat) -> CGFloat {
        func getComponentLuminance(component: CGFloat) -> CGFloat {
            return component <= 0.03928
            ? component / 12.92
            : pow((component + 0.055) / 1.055, 2.4)
        }
        let rLuminance = getComponentLuminance(component: red)
        let gLuminance = getComponentLuminance(component: green)
        let bLuminance = getComponentLuminance(component: blue)
        return 0.2126 * rLuminance + 0.7152 * gLuminance + 0.0722 * bLuminance;
    }
    
    func addContrast(component: CGFloat, darkMode: Bool) -> CGFloat {
        if darkMode {
            return min(1.0, (component + 0.12) * 1.2)
        }
        else {
            return max(0.0, (component - 0.12) / 1.2)
        }
    }

    public func lighten(by percentage: CGFloat = 1.0) -> NSColor {
        guard let components = self.getRGBComponents() else { return self }
        return NSColor(red: components.red,
                       green: components.green,
                       blue: components.blue,
                      alpha: percentage)
     }
}
