//
//  ColorGraphicsView.swift
//  Color Picker Plus
//
//  Created by Viktor Hundahl Strate on 13/06/2018.
//

import Cocoa

protocol ChangeColorDelegate: class {
    /// Called on every drag event
    func colorChanged(color: HSV)

    /// Called when mouse is released
    func colorSettled(color: HSV)
}


final class ColorGraphicsView: NSView {
    
    override func prepareForInterfaceBuilder() {
        currentColor = HSV(h: 40, s: 0.8, v: 0.7)
        selectedHSBComponent = .brightness
    }
    
    weak var delegate: ChangeColorDelegate?
    var selectedSlider: Sliders = .None
    var selectedHSBComponent: HSBComponent = .brightness {
        didSet {
            needsDisplay = true
        }
    }
    
    enum Sliders {
        case None
        case Main
        case Secondary
        case Alpha
    }
    
    var currentColor: HSV = HSV(h: 0, s: 1, v: 1) {
        didSet {
            needsDisplay = true
        }
    }

    override func draw(_ dirtyRect: NSRect) {
        super.draw(dirtyRect)
        
        let context = NSGraphicsContext.current!.cgContext
        
        drawMainView(context)
        drawMainCircleIndicator(context)
        
        //drawAlphaSlider(context)
        drawSecondarySlider(context)
        
        
    }
    
    fileprivate struct Constants {
        static let bottomSliderHeight: CGFloat = 16.0
        static let verticalMargin: CGFloat = 10.0
    }

    // Rects
    
    private func totalRect() -> NSRect {
        return bounds
    }
    
    private func mainViewRect() -> NSRect {
        
        let total = totalRect()
        
        let bottomMargin: CGFloat = Constants.bottomSliderHeight + Constants.verticalMargin
        let height = total.height - bottomMargin
        let smallestSize = total.width
        return NSRect(x: total.minX, y: total.minY + bottomMargin, width: smallestSize, height: height)
    }
    
    private func alphaSliderRect() -> NSRect {
        
        let mainRect = mainViewRect()
        
        return NSRect(x: totalRect().minX, y: mainRect.minY - (Constants.verticalMargin * 2 + Constants.bottomSliderHeight * 2), width: mainRect.width, height: Constants.bottomSliderHeight)
    }
    
    private func secondarySliderRect() -> NSRect {
        
        let mainRect = mainViewRect()
        
        return NSRect(x: totalRect().minX, y: mainRect.minY - (Constants.verticalMargin + Constants.bottomSliderHeight), width: mainRect.width, height: Constants.bottomSliderHeight)
    }
    
    // MARK: Draw functions
    private var hsbSquare: CGImage?
    private var hsbSquareColor: HSV = HSV(h: 0, s: 1, v: 1)
    private var hsbSquarePreviousComponet: HSBComponent? = nil
    
    private func drawMainView(_ context: CGContext) {
        
        if (hsbSquare == nil || hsbSquarePreviousComponet == nil ||
            hsbSquarePreviousComponet != selectedHSBComponent || hsbSquareColor.h != currentColor.h) {
            
            hsbSquareColor = currentColor
            
            switch (selectedHSBComponent) {
            case .hue:                hsbSquare = HSBGen.createSaturationBrightnessSquareContentImageWithHue(hue: currentColor.h)
                break
            case .saturation:
                hsbSquare = HSBGen.createHueBrightnessSquareContentImageWithSaturation(saturation: currentColor.s)
                break
            case .brightness:
                hsbSquare = HSBGen.createHueSaturationSquareContentImageWithBrightness(brightness: currentColor.v)
                break
            }
            
        }

        context.draw(hsbSquare!, in: mainViewRect())
    }

    // MARK: Secondary slider functions
    
    private func secondaryPointingArrowOrigin() -> CGPoint {
        let sliderRect = secondarySliderRect()
        
        var x: CGFloat = 0
        
        switch (selectedHSBComponent) {
        case .hue:
            x = currentColor.h / 360 * sliderRect.width + totalRect().minX
            break
        case .saturation:
            x = currentColor.s * sliderRect.width + totalRect().minX
            break
        case .brightness:
            x = currentColor.v * sliderRect.width + totalRect().minX
            break
        }
        
        return CGPoint(x: x, y: sliderRect.maxY)
    }
    
    private func drawSecondarySlider(_ context: CGContext) {
        
        let sliderRect = secondarySliderRect()
        
        let bar = HSBGen.createHSVBarContentImage(hsbComponent: selectedHSBComponent, hsv: currentColor)!
        context.draw(bar, in: sliderRect)
        
        drawPointingArrow(context, position: secondaryPointingArrowOrigin())
    }
    
    // MARK: Alpha slider functions
    
    private func alphaPointingArrowOrigin() -> CGPoint {
        let alphaRec = alphaSliderRect()
        
        let x = currentColor.a * alphaRec.width + totalRect().minX
        
        return CGPoint(x: x, y: alphaRec.maxY)
    }
    
    private func drawAlphaSlider(_ context: CGContext) {
        let colorSpace = CGColorSpaceCreateDeviceRGB()
        
        let color = currentColor.toNSColor().withAlphaComponent(1).cgColor
        
        let colors = [color.copy(alpha: 0), color] as CFArray
        let alphaRec = alphaSliderRect()
        
        ColorGraphicsView.drawTransparentGridOverlay(rect: alphaRec, context: context)
        
        let gradient = CGGradient(colorsSpace: colorSpace, colors: colors, locations: nil)!
        
        context.clip(to: alphaRec)
        
        context.drawLinearGradient(gradient, start: CGPoint(x: alphaRec.minX, y: alphaRec.maxY), end: CGPoint(x: alphaRec.maxX, y: alphaRec.maxY), options: CGGradientDrawingOptions.drawsBeforeStartLocation)
        
        context.resetClip()
        
        drawPointingArrow(context, position: alphaPointingArrowOrigin())
    }
    
    private func drawMainCircleIndicator(_ context: CGContext) {
        
        let viewRect = mainViewRect()
        
        var x: CGFloat = 0
        var y: CGFloat = 0
        
        switch (selectedHSBComponent) {
        case .hue:
            x = currentColor.s * viewRect.width + totalRect().minX
            y = currentColor.v * viewRect.height
            break
        case .saturation:
            x = currentColor.h / 360 * viewRect.width + totalRect().minX
            y = currentColor.v * viewRect.height
            break
        case .brightness:
            x = currentColor.h / 360 * viewRect.width + totalRect().minX
            y = currentColor.s * viewRect.height
            break
        }
        
        context.clip(to: viewRect)
        
        NSColor.white.setStroke()
        context.addEllipse(in: CGRect(x: x - 4, y: y - 4 + viewRect.minY, width: 8, height: 8))
        context.strokePath()
        
        context.resetClip()
    }
    
    private func pointingArrowBountingRect(position: CGPoint) -> CGRect {
        let size: CGFloat = 0.25
        
        let offset = CGPoint(x: position.x, y: position.y - 30 * size - 6)
        
        let xMargin = size * 25
        let yMargin = size * 30
        
        return CGRect(x: offset.x - xMargin, y: offset.y - yMargin, width: xMargin * 2, height: yMargin * 2)
        
    }
    
    private func drawPointingArrow(_ context: CGContext, position: CGPoint) {
        
        let size: CGFloat = 0.25
        
        let pos = CGPoint(x: position.x, y: position.y - 30 * size - 6)
        
        context.beginPath()
        context.move(to: CGPoint(x: pos.x - 15 * size, y: pos.y - 30 * size))
        context.addLine(to: CGPoint(x: pos.x + 15 * size, y: pos.y - 30 * size))
        
        context.addArc(center: CGPoint(x: pos.x + 15 * size, y: pos.y - 20 * size), radius: 10 * size, startAngle: CGFloat.pi * 1.5, endAngle: 0, clockwise: false)
        
        context.addLine(to: CGPoint(x: pos.x + 25 * size, y: pos.y))
        context.addLine(to: CGPoint(x: pos.x, y: pos.y + 30 * size))
        context.addLine(to: CGPoint(x: pos.x - 25 * size, y: pos.y))
        context.addLine(to: CGPoint(x: pos.x - 25 * size, y: pos.y - 20 * size))
        
        context.addArc(center: CGPoint(x: pos.x - 15 * size, y: pos.y - 20 * size), radius: 10 * size, startAngle: CGFloat.pi, endAngle: CGFloat.pi * 1.5, clockwise: false)
        
        context.closePath()
        
        NSColor.black.withAlphaComponent(0.1).setStroke()
        NSColor.white.setFill()
        
        context.drawPath(using: CGPathDrawingMode.fillStroke)
    }
    
    static func drawTransparentGridOverlay(rect: NSRect, context: CGContext) {
        let squareSize: CGFloat = 8
        
        context.clip(to: rect)
        
        for x in 0...Int(rect.width / squareSize) {
            for y in 0...Int(rect.height / squareSize) {
                let evenX = x % 2 == 0
                let evenY = y % 2 == 0
                let even = (!evenX && evenY) || (evenX && !evenY)
                
                if (even) {
                    NSColor.darkGray.setFill()
                } else {
                    NSColor.lightGray.setFill()
                }
                
                context.fill(CGRect(x: rect.minX + CGFloat(x) * squareSize, y: rect.minY + CGFloat(y) * squareSize, width: squareSize, height: squareSize))
            }
        }
        
        context.resetClip()
    }
}

// Mouse events
extension ColorGraphicsView {
    override func mouseDown(with event: NSEvent) {
        
        guard let localPoint = window?.contentView?.convert(event.locationInWindow, to: self) else {
            return
        }
        
        let secondaryHandle = pointingArrowBountingRect(position: secondaryPointingArrowOrigin())
        let alphaHandle = pointingArrowBountingRect(position: alphaPointingArrowOrigin())
        
        if (mainViewRect().contains(localPoint)) {
            selectedSlider = .Main
            updateMainCursor(locationInWindow: event.locationInWindow)
            
        } else if (secondarySliderRect().contains(localPoint) || secondaryHandle.contains(localPoint)) {
            selectedSlider = .Secondary
            updateSecondaryCursor(locationInWindow: event.locationInWindow)
            
        } else if (alphaSliderRect().contains(localPoint) || alphaHandle.contains(localPoint)) {
            selectedSlider = .Alpha
            updateAlphaCursor(locationInWindow: event.locationInWindow)
        } else {
            selectedSlider = .None
        }
    }
    
    override func mouseDragged(with event: NSEvent) {
        
        if (selectedSlider == .Main) {
            updateMainCursor(locationInWindow: event.locationInWindow)
            
        } else if (selectedSlider == .Secondary) {
            updateSecondaryCursor(locationInWindow: event.locationInWindow)
            
        } else if (selectedSlider == .Alpha) {
            updateAlphaCursor(locationInWindow: event.locationInWindow)
            
        }
        
    }
    
    override func mouseUp(with event: NSEvent) {
        self.delegate?.colorSettled(color: currentColor)
    }
    
    private func updateMainCursor(locationInWindow: NSPoint) {
        var newColor = currentColor
        let mainWindowRect = convert(mainViewRect(), to: window?.contentView)
        
        var x = (locationInWindow.x - mainWindowRect.minX) / mainWindowRect.width
        var y = (locationInWindow.y - mainWindowRect.minY) / mainWindowRect.height
        
        x = min(1, x)
        x = max(0, x)
        
        y = min(1, y)
        y = max(0, y)
        
        switch (selectedHSBComponent) {
        case .hue:
            newColor.s = x
            newColor.v = y
            break
        case .saturation:
            newColor.h = round(x * 360)
            newColor.v = y
            break
        case .brightness:
            newColor.h = round(x * 360)
            newColor.s = y
            break
        }
        
        currentColor = newColor
        delegate?.colorChanged(color: newColor)
    }
    
    private func updateSecondaryCursor(locationInWindow: NSPoint) {
        var newColor = currentColor
        let secondaryWindowRect = convert(secondarySliderRect(), to: window?.contentView)
        
        var x = (locationInWindow.x - secondaryWindowRect.minX) / secondaryWindowRect.width
        
        x = min(1, x)
        x = max(0, x)
        
        switch (selectedHSBComponent) {
        case .hue:
            newColor.h = round(x * 360)
            break
        case .saturation:
            newColor.s = x
            break
        case .brightness:
            newColor.v = x
            break
        }
        
        currentColor = newColor
        delegate?.colorChanged(color: newColor)
    }
    
    private func updateAlphaCursor(locationInWindow: NSPoint) {
        var newColor = currentColor
        let alphaWindowRect = convert(alphaSliderRect(), to: window?.contentView)
        
        var x = (locationInWindow.x - alphaWindowRect.minX) / alphaWindowRect.width
        
        x = min(1, x)
        x = max(0, x)
        
        newColor.a = x
        
        currentColor = newColor
        delegate?.colorChanged(color: newColor)
    }
}
