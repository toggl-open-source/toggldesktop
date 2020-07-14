//
//  NSColor+VisibleColor.swift
//  TogglTrack
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

    public func lighten(by percentage: CGFloat = 1.0) -> NSColor {
        guard let components = self.getRGBComponents() else { return NSColor(deviceWhite: 0, alpha: percentage) }
        return NSColor(red: components.red,
                       green: components.green,
                       blue: components.blue,
                      alpha: percentage)
     }
}
