// This file is based on the Huekit project by louisdh
// Which is licensed under the MIT Licese
//
// https://github.com/louisdh/huekit

import Foundation
import AppKit

public struct HSV: Hashable {
    /// In degrees (range 0...360)
    public var h: CGFloat
    
    /// Percentage in range 0...1
    public var s: CGFloat
    
    /// Percentage in range 0...1
    /// Also known as "brightness" (B)
    public var v: CGFloat
    
    public var a: CGFloat
    
    init (h: CGFloat, s: CGFloat, v: CGFloat) {
        self.h = h
        self.s = s
        self.v = v
        self.a = 1
    }
    
    init (h: CGFloat, s: CGFloat, v: CGFloat, a: CGFloat) {
        self.init(h: h, s: s, v: v)
        self.a = 1
    }
}

extension HSV {
    
    init (color: NSColor) {
        
        var convertedColor: NSColor!
        
        if (color.colorSpace != .genericRGB) {
            convertedColor = color.usingColorSpace(NSColorSpace.genericRGB)!
        } else {
            convertedColor = color
        }
        
        self.h = convertedColor.hueComponent * 360
        self.s = convertedColor.saturationComponent
        self.v = convertedColor.brightnessComponent
        self.a = convertedColor.alphaComponent
    }
    
    func toNSColor() -> NSColor {
        return NSColor(hue: h / 360, saturation: s, brightness: v, alpha: a)
    }
    
    func toRGB() -> RGB {
        var i: Int
        var f: CGFloat, p: CGFloat, q: CGFloat, t: CGFloat
        
        if (self.s == 0) {
            // Gray
            return RGB(r: self.v, g: self.v, b: self.v)
        }
        
        let hSector = self.h / 60.0
        i = Int(exactly: Float(floor(hSector)))!
        f = hSector - CGFloat(i)
        p = self.v * (1 - self.s)
        q = self.v * (1 - self.s * f)
        t = self.v * (1 - self.s * (1 - f))
        
        switch i {
        case 0:
            return RGB(r: v, g: t, b: p)
        case 1:
            return RGB(r: q, g: v, b: p)
        case 2:
            return RGB(r: p, g: v, b: t)
        case 3:
            return RGB(r: p, g: q, b: v)
        case 4:
            return RGB(r: t, g: p, b: v)
        default: // case 5
            return RGB(r: v, g: p, b: q)
        }
    }
    
    func equals(hsv other: HSV) -> Bool {

        if self.h.rounded() != other.h.rounded() {
            return false
        }
        
        if (self.s * 100).rounded() != (other.s * 100).rounded() {
            return false
        }

        if (self.v * 100).rounded() != (other.v * 100).rounded() {
            return false
        }
        
        if (self.a * 100).rounded() != (other.a * 100).rounded() {
            return false
        }
        
        return true
    }

    static func hueToRGB(hue h: CGFloat) -> RGB {
        
        let hPrime = h / 60.0
        
        let x = 1.0 - abs(hPrime.truncatingRemainder(dividingBy: 2.0) - 1.0)
        
        let r: CGFloat
        let g: CGFloat
        let b: CGFloat
        
        if hPrime < 1.0 {
            
            r = 1
            g = x
            b = 0
            
        } else if hPrime < 2.0 {
            
            r = x
            g = 1
            b = 0
            
        } else if hPrime < 3.0 {
            
            r = 0
            g = 1
            b = x
            
        } else if hPrime < 4.0 {
            
            r = 0
            g = x
            b = 1
            
        } else if hPrime < 5.0 {
            
            r = x
            g = 0
            b = 1
            
        } else {
            
            r = 1
            g = 0
            b = x
            
        }
        
        return RGB(r: r, g: g, b: b)
    }
}
