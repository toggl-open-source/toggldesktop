// This file is based on the Huekit project by louisdh
// Which is licensed under the MIT Licese
//
// https://github.com/louisdh/huekit

import Foundation
import AppKit

public struct RGB: Hashable {
    /// In range 0...1
    public var r: CGFloat
    
    /// In range 0...1
    public var g: CGFloat
    
    /// In range 0...1
    public var b: CGFloat
}

public extension RGB {
    init (color: NSColor) {
        
        var convertedColor: NSColor!
        
        if (color.colorSpace != .genericRGB) {
            convertedColor = color.usingColorSpace(NSColorSpace.genericRGB)!
        } else {
            convertedColor = color
        }
        
        r = convertedColor.redComponent
        g = convertedColor.greenComponent
        b = convertedColor.blueComponent
    }
}

public extension RGB {
    func toHEX() -> String {
        
        let hexR = convertNumberToHex(number: r)
        let hexG = convertNumberToHex(number: g)
        let hexB = convertNumberToHex(number: b)
        
        return hexR + hexG + hexB
    }
    
    func toNSColor() -> NSColor {
        return NSColor(red: r, green: g, blue: b, alpha: 1.0)
    }
    
    static func fromHEX(_ string: NSString) -> RGB? {
        if (string.length != 6) {
            return nil
        }
        
        let r = convertHexToNumber(hex: string.substring(with: NSRange(location: 0, length: 2)))
        let g = convertHexToNumber(hex: string.substring(with: NSRange(location: 2, length: 2)))
        let b = convertHexToNumber(hex: string.substring(with: NSRange(location: 4, length: 2)))
        
        if (r == nil || g == nil || b == nil) {
            return nil
        }
        
        let newRGB = RGB(r: CGFloat(r!) / 255.0, g: CGFloat(g!) / 255.0, b: CGFloat(b!) / 255.0)
        
        return newRGB
    }
    
    fileprivate func convertNumberToHex(number: CGFloat) -> String {
        var strNum = String(Int(number * 255), radix: 16)
        
        if (strNum.count < 2) {
            strNum = "0" + strNum
        }
        
        return strNum
    }
    
    fileprivate static func convertHexToNumber(hex string: String) -> Int? {
        return Int(string, radix: 16)
    }
    
}

public extension RGB {
    
    func toHSV() -> HSV {
        let minimum: CGFloat = min(r, g, b)
        let maximum: CGFloat = max(r, g, b)
        
        let delta: CGFloat = maximum - minimum
        
        var hue: CGFloat
        var saturation: CGFloat
        let value: CGFloat = maximum
        
        // Find saturation
        if (maximum != 0) {
            saturation = delta / maximum
        } else {
            // r = g = b = 0 (black), hue is undefined
            saturation = 0
        }
        
        // Find hue
        if (delta != 0) {
            if (r == maximum) {
                hue = (g - b) / delta
                
            } else if (g == maximum) {
                hue = 2 + (b - r) / delta
                
            } else {
                hue = 4 + (r - g) / delta
            }
        } else {
            hue = 0
        }
        
        hue *= 60
        
        if (hue < 0) {
            hue += 360
        }
        
        return HSV(h: hue, s: saturation, v: value)
        
    }
    
}
