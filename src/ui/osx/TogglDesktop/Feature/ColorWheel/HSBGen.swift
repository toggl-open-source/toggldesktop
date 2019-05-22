// This file is based on the Huekit project by louisdh
// Which is licensed under the MIT Licese
//
// https://github.com/louisdh/huekit

import Foundation
import CoreGraphics

enum HSBComponent: Int {
    case hue = 0
    case saturation = 1
    case brightness = 2
}

class HSBGen {
    
    static func createBGRxImageContext(w: Int, h: Int) -> CGContext? {
        
        let colorSpace = CGColorSpaceCreateDeviceRGB()
        
        // BGRA is the most efficient on the iPhone.
        var bitmapInfo = CGBitmapInfo(rawValue: CGImageByteOrderInfo.order32Little.rawValue)
        
        let noneSkipFirst = CGBitmapInfo(rawValue: CGImageAlphaInfo.noneSkipFirst.rawValue)
        
        bitmapInfo.formUnion(noneSkipFirst)
        
        let context = CGContext(data: nil, width: w, height: h, bitsPerComponent: 8, bytesPerRow: w * 4, space: colorSpace, bitmapInfo: bitmapInfo.rawValue)
        
        return context
    }
    
    /// Generates an image where the specified barComponentIndex (0=H, 1=S, 2=V)
    /// varies across the x-axis of the 256x1 pixel image and the other components
    /// remain at the constant value specified in the hsv array.
    static func createHSVBarContentImage(hsbComponent: HSBComponent, hsv: HSV) -> CGImage? {
        
        guard let context = createBGRxImageContext(w: 256, h: 1) else {
            return nil
        }
        
        guard var ptr = context.data?.assumingMemoryBound(to: UInt8.self) else {
            return nil
        }
        
        for x in 0..<256 {
            
            var hsvVal = hsv
            
            switch (hsbComponent) {
            case .hue:
                hsvVal = HSV(h: CGFloat(x) / 255 * 360, s: 1, v: 1)
                break
            case .saturation:
                hsvVal.s = CGFloat(x) / 255.0
            case .brightness:
                hsvVal.v = CGFloat(x) / 255.0
            }
            
            let rgb = hsvVal.toRGB()
            
            ptr[0] = UInt8(rgb.b * 255.0)
            ptr[1] = UInt8(rgb.g * 255.0)
            ptr[2] = UInt8(rgb.r * 255.0)
            
            ptr = ptr.advanced(by: 4)
        }
        
        let image = context.makeImage()
        
        return image
    }
    
    static private func blend(_ value: UInt, _ percentIn255: UInt) -> UInt {
        return (value) * (percentIn255) / 255
    }
    
    // Generates a 256x256 image with the specified constant hue where the
    // Saturation and value vary in the X and Y axes respectively.
    static func createSaturationBrightnessSquareContentImageWithHue(hue: CGFloat) -> CGImage? {
        
        guard let context = createBGRxImageContext(w: 256, h: 256) else {
            return nil
        }
        
        guard var dataPtr = context.data?.assumingMemoryBound(to: UInt8.self) else {
            return nil
        }
        
        let rowBytes = context.bytesPerRow
        
        //let hsv = HSV(h: hue, s: 0, v: 0)
        //let rgb = hsv.hueToRGB()
        let rgb = HSV.hueToRGB(hue: hue)
        
        let r = rgb.r
        let g = rgb.g
        let b = rgb.b
        
        let r_s = (UInt) ((1.0 - r) * 255)
        let g_s = (UInt) ((1.0 - g) * 255)
        let b_s = (UInt) ((1.0 - b) * 255)
        
        // This doesn't use Swift ranges because those are pretty slow in debug builds
        
        // Width / Saturation
        var s: UInt = 0
        
        while true {
            
            // Every row
            
            var ptr = dataPtr
            
            let r_hs: UInt = 255 - blend(s, r_s)
            let g_hs: UInt = 255 - blend(s, g_s)
            let b_hs: UInt = 255 - blend(s, b_s)
            
            // Height / Value
            var v: UInt = 255
            
            while true {
                
                // Every column
                
                // Really, these should all be of the form used in blend(),
                // which does a divide by 255. However, integer divide is
                // implemented in software on ARM, so a divide by 256
                // (done as a bit shift) will be *nearly* the same value,
                // and is faster. The more-accurate versions would look like:
                //    ptr[0] = blend(v, b_hs);
                
                ptr[0] = UInt8((v * b_hs) >> 8)
                ptr[1] = UInt8((v * g_hs) >> 8)
                ptr[2] = UInt8((v * r_hs) >> 8)
                
                ptr = ptr.advanced(by: rowBytes)
                
                if v == 0 {
                    break
                }
                
                v -= 1
            }
            
            dataPtr = dataPtr.advanced(by: 4)
            
            if s == 255 {
                break
            }
            
            s += 1
        }
        
        let image = context.makeImage()
        
        return image
    }
    
    // Generates a 256x256 image with the specified constant saturation where the
    // Hue and value vary in the X and Y axes respectively.
    static func createHueBrightnessSquareContentImageWithSaturation(saturation: CGFloat) -> CGImage? {
        
        guard let context = createBGRxImageContext(w: 256, h: 256) else {
            return nil
        }
        
        guard var dataPtr = context.data?.assumingMemoryBound(to: UInt8.self) else {
            return nil
        }
        
        let rowBytes = context.bytesPerRow
        
        // Width / Hue
        var h: UInt = 0
        
        while true {
            
            // Every row
            var ptr = dataPtr
            
            let rgb = HSV(h: CGFloat(h) / 255.0 * 360.0, s: saturation, v: 1.0).toRGB()
            
            let r: UInt = UInt(rgb.r * 255)
            let g: UInt = UInt(rgb.g * 255)
            let b: UInt = UInt(rgb.b * 255)
            
            // Height / Value
            var v: UInt = 255
            
            while true {
                
                // Performance "hack"
                ptr[0] = UInt8((v * b) >> 8)
                ptr[1] = UInt8((v * g) >> 8)
                ptr[2] = UInt8((v * r) >> 8)
                
                ptr = ptr.advanced(by: rowBytes)
                
                if v == 0 {
                    break
                }
                
                v -= 1
            }
            
            dataPtr = dataPtr.advanced(by: 4)
            
            if h == 255 {
                break
            }
            
            h += 1
        }
        
        let image = context.makeImage()
        
        return image
    }
    
    // Generates a 256x256 image with the specified constant saturation where the
    // Hue and value vary in the X and Y axes respectively.
    static func createHueSaturationSquareContentImageWithBrightness(brightness: CGFloat) -> CGImage? {
        
        guard let context = createBGRxImageContext(w: 256, h: 256) else {
            return nil
        }
        
        guard var dataPtr = context.data?.assumingMemoryBound(to: UInt8.self) else {
            return nil
        }
        
        let rowBytes = context.bytesPerRow
        
        // Width / Hue
        var h: UInt = 0
        
        while true {
            
            // Every row
            var ptr = dataPtr
            
            let rgb = HSV(h: CGFloat(h) / 255.0 * 360.0, s: 1, v: brightness).toRGB()
            
            /*let r: UInt = UInt(rgb.r * 255)
            let g: UInt = UInt(rgb.g * 255)
            let b: UInt = UInt(rgb.b * 255)*/
            
            let highest = max(rgb.r, rgb.g, rgb.b)
            
            // Height / Saturation
            var s: UInt = 255
            
            while true {
                
                let satPart = 1 - (CGFloat(s) / 255)
                
                let r: UInt = UInt((rgb.r + satPart * (highest - rgb.r)) * 255)
                let g: UInt = UInt((rgb.g + satPart * (highest - rgb.g)) * 255)
                let b: UInt = UInt((rgb.b + satPart * (highest - rgb.b)) * 255)
                
                // Performance "hack"
                ptr[0] = UInt8((b * 255) >> 8)
                ptr[1] = UInt8((g * 255) >> 8)
                ptr[2] = UInt8((r * 255) >> 8)
                
                ptr = ptr.advanced(by: rowBytes)
                
                if s == 0 {
                    break
                }
                
                s -= 1
            }
            
            dataPtr = dataPtr.advanced(by: 4)
            
            if h == 255 {
                break
            }
            
            h += 1
        }
        
        let image = context.makeImage()
        
        return image
    }
    
}
