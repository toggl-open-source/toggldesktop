//
//  CornerBoxView.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 2/18/20.
//  Copyright © 2020 Alari. All rights reserved.
//

import Cocoa

@IBDesignable
final class CornerBoxView: NSView {

    // MARK: Variables

    @IBInspectable var cornerRadius: CGFloat = 0 {
        didSet {
            layer?.cornerRadius = cornerRadius
        }
    }
    @IBInspectable var borderWidth: CGFloat = 0 {
        didSet {
            layer?.borderWidth = borderWidth
        }
    }
    @IBInspectable var borderColor: NSColor = NSColor.black {
        didSet {
            layer?.borderColor = borderColor.cgColor
        }
    }
    @IBInspectable var backgroundColor: NSColor = NSColor.lightGray {
        didSet {
            layer?.backgroundColor = backgroundColor.cgColor
        }
    }

    var corners: Corners = [.bottomLeft, .bottomRight, .topLeft, .topRight] {
        didSet {
            initCornerMask()
        }
    }

    // MARK: Public

    override init(frame frameRect: NSRect) {
        super.init(frame: frameRect)
        initCommon()
        initCornerMask()
    }

    required init?(coder: NSCoder) {
        super.init(coder: coder)
        initCommon()
        initCornerMask()
    }

    private func initCommon() {
        wantsLayer = true
        layer?.masksToBounds = true
    }

    private func initCornerMask() {
        if #available(OSX 10.13, *) {
            layer?.maskedCorners = corners.layerCornerMask
        } else {
            let path = NSBezierPath(rect: bounds, roundedCorners: corners, cornerRadius: cornerRadius)
            let mask = CAShapeLayer()
            mask.path = path.cgPath
            layer?.mask = mask
        }
    }
}
