//
//  BezierPath+Corner.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 2/26/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

var str = "Hello, playground"

struct Corners: OptionSet {
    let rawValue: Int

    init(rawValue: Int) {
        self.rawValue = rawValue
    }

    static let topLeft = Corners(rawValue: 1 << 0)
    static let bottomLeft = Corners(rawValue: 1 << 1)
    static let topRight = Corners(rawValue: 1 << 2)
    static let bottomRight = Corners(rawValue: 1 << 3)

    func flipped() -> Corners {
        var flippedCorners: Corners = []

        if contains(.bottomRight) {
            flippedCorners.insert(.topRight)
        }

        if contains(.topRight) {
            flippedCorners.insert(.bottomRight)
        }

        if contains(.bottomLeft) {
            flippedCorners.insert(.topLeft)
        }

        if contains(.topLeft) {
            flippedCorners.insert(.bottomLeft)
        }

        return flippedCorners
    }

    var layerCornerMask: CACornerMask {
        var mask: CACornerMask = []

        if contains(.topLeft) {
            mask.insert(.layerMinXMaxYCorner)
        }

        if contains(.topRight) {
            mask.insert(.layerMaxXMaxYCorner)
        }

        if contains(.bottomLeft) {
            mask.insert(.layerMinXMinYCorner)
        }

        if contains(.bottomRight) {
            mask.insert(.layerMaxXMinYCorner)
        }

        return mask
    }
}

extension NSBezierPath {

    // Compatibility bewteen NSBezierPath and UIBezierPath

    #if os(iOS) || os(tvOS)
    func curve(to point: CGPoint, controlPoint1: CGPoint, controlPoint2: CGPoint) {
        addCurve(to: point, controlPoint1: controlPoint1, controlPoint2: controlPoint2)
    }

    func line(to point: CGPoint) {
        addLine(to: point)
    }
    #endif

    convenience init(rect: CGRect, roundedCorners: Corners, cornerRadius: CGFloat) {
        self.init()

        // On iOS & tvOS, we need to flip the corners
        #if os(iOS) || os(tvOS)
        let corners = roundedCorners.flipped()
        #elseif os(macOS)
        let corners = roundedCorners
        #endif

        let maxX: CGFloat = rect.size.width
        let minX: CGFloat = 0
        let maxY: CGFloat = rect.size.height
        let minY: CGFloat =  0

        let bottomRightCorner = CGPoint(x: maxX, y: minY)

        move(to: bottomRightCorner)

        if corners.contains(.bottomRight) {
            line(to: CGPoint(x: maxX - cornerRadius, y: minY))
            curve(to: CGPoint(x: maxX, y: minY + cornerRadius), controlPoint1: bottomRightCorner, controlPoint2: bottomRightCorner)
        }
        else {
            line(to: bottomRightCorner)
        }

        let topRightCorner = CGPoint(x: maxX, y: maxY)

        if corners.contains(.topRight) {
            line(to: CGPoint(x: maxX, y: maxY - cornerRadius))
            curve(to: CGPoint(x: maxX - cornerRadius, y: maxY), controlPoint1: topRightCorner, controlPoint2: topRightCorner)
        }
        else {
            line(to: topRightCorner)
        }

        let topLeftCorner = CGPoint(x: minX, y: maxY)

        if corners.contains(.topLeft) {
            line(to: CGPoint(x: minX + cornerRadius, y: maxY))
            curve(to: CGPoint(x: minX, y: maxY - cornerRadius), controlPoint1: topLeftCorner, controlPoint2: topLeftCorner)
        }
        else {
            line(to: topLeftCorner)
        }

        let bottomLeftCorner = CGPoint(x: minX, y: minY)

        if corners.contains(.bottomLeft) {
            line(to: CGPoint(x: minX, y: minY + cornerRadius))
            curve(to: CGPoint(x: minX + cornerRadius, y: minY), controlPoint1: bottomLeftCorner, controlPoint2: bottomLeftCorner)
        }
        else {
            line(to: bottomLeftCorner)
        }
    }
}

extension NSBezierPath {

    var cgPath: CGPath {
        get { return self.transformToCGPath() }
    }

    /// Transforms the NSBezierPath into a CGPath
    ///
    /// :returns: The transformed NSBezierPath
    private func transformToCGPath() -> CGPath {

        // Create path
        let path = CGMutablePath()
        let points = UnsafeMutablePointer<NSPoint>.allocate(capacity: 3)
        let numElements = self.elementCount

        if numElements > 0 {

            var didClosePath = true

            for index in 0..<numElements {

                let pathType = self.element(at: index, associatedPoints: points)

                switch pathType {
                case .moveTo:
                    path.move(to: CGPoint(x: points[0].x, y: points[0].y))
                case .lineTo:
                    path.addLine(to: CGPoint(x: points[0].x, y: points[0].y))
                    didClosePath = false
                case .curveTo:
                    path.addCurve(to: CGPoint(x: points[0].x, y: points[0].y), control1: CGPoint(x: points[1].x, y: points[1].y), control2: CGPoint(x: points[2].x, y: points[2].y))
                    didClosePath = false
                case .closePath:
                    path.closeSubpath()
                    didClosePath = true
                default:
                    break
                }
            }

            if !didClosePath { path.closeSubpath() }
        }

        points.deallocate()
        return path
    }
}
