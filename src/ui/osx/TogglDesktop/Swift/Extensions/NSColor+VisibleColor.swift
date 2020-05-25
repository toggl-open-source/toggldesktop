//
//  NSColor+VisibleColor.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 8/12/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

extension NSColor {

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
        guard let components = self.getRGBComponents() else { return NSColor(deviceWhite: 0, alpha: percentage) }
        return NSColor(red: components.red,
                       green: components.green,
                       blue: components.blue,
                      alpha: percentage)
     }
}
