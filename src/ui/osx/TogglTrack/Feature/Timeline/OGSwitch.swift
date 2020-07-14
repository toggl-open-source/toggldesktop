//
//  OGSwitch.swift
//  OGSwitch
//
//  Created by Oskar Groth on 2017-02-22.
//  Copyright © 2017 Oskar Groth. All rights reserved.
//

import Cocoa
import QuartzCore

//Can not be NSControl due to 10.9 issues: https://github.com/iluuu1994/ITSwitch/issues/21

@IBDesignable public class OGSwitch: NSControl {
    
    public enum Size {
        
        case small
        case medium
        case large
        
        public var size: CGSize {
            switch self {
            case .small:
                return CGSize(width: 46, height: 28)
            case .medium:
                return CGSize(width: 56, height: 34)
            case .large:
                return CGSize(width: 69, height: 41)
            }
        }
        
    }
    
    @IBInspectable public var tintColor: NSColor = NSColor(calibratedRed:0.27, green: 0.86, blue: 0.36, alpha: 1.0) {
        didSet {
            reloadLayer()
        }
    }
    @IBInspectable public var knobBackgroundColor: NSColor = NSColor(calibratedWhite:1.0, alpha: 1.0) {
        didSet {
            reloadLayer()
        }
    }
    @IBInspectable public var disabledBorderColor: NSColor = NSColor(calibratedWhite: 0.0, alpha: 0.2) {
        didSet {
            reloadLayer()
        }
    }
    @IBInspectable public var disabledBackgroundColor: NSColor = NSColor.clear  {
        didSet {
            reloadLayer()
        }
    }
    @IBInspectable public var inactiveBackgroundColor: NSColor = NSColor(calibratedWhite: 0.0, alpha:0.3) {
        didSet {
            reloadLayer()
        }
    }
    @IBInspectable public var animationDuration: TimeInterval = 0.4
    @IBInspectable public var inactiveIcon: NSImage?  {
        didSet {
            reloadLayer()
        }
    }
    @IBInspectable public var activeIcon: NSImage?  {
        didSet {
            reloadLayer()
        }
    }
    @IBInspectable public var ignoreRatio: Bool = true {
        didSet {
            reloadLayer()
        }
    }
    override public var isEnabled: Bool {
        didSet {
            reloadLayerAnimated(animated: true)
        }
    }
    
    lazy var kBorderLineWidth: CGFloat = bounds.height*(2/31)
    
    let kGoldenRatio:CGFloat = 1.6180339875
    let kDecreasedGoldenRatio:CGFloat = 1.38
    let kEnabledOpacity:Float = 1.0
    let kDisabledOpacity:Float = 0.5
    var dragEvents = 0
    public var isOn:Bool = false
    public var isActive:Bool = false
    public var hasDragged:Bool  = false
    public var isDraggingTowardsOn:Bool = false
    public var rootLayer:CALayer?
    public var backgroundLayer:CALayer?
    public var knobLayer: CALayer?
    public var knobInsideLayer: CALayer?
    public var iconLayer = CALayer()
    public var lockInteraction: Bool = false
    
    override public var acceptsFirstResponder: Bool {
        get {
            return true
        }
    }
    
    required public init?(coder:NSCoder) {
        super.init(coder: coder)
        setup()
    }
    
    convenience public init(size: CGSize) {
        self.init(frame: NSRect(origin: .zero, size: size))
    }
    
    override public init(frame:NSRect) {
        super.init(frame: frame);
        setup()
    }
    
    internal func setup() {
        isEnabled = true
        setupLayers()
        reloadLayerSize()
        setupIcon()
        reloadLayer()
    }
        
    public func setOn(isOn: Bool, animated: Bool) {
        if self.isOn != isOn {
            self.isOn = isOn
            reloadLayerAnimated(animated: animated)
        }
    }
    
    internal func setupIcon() {
        guard let icon = inactiveIcon, let bounds = knobLayer?.bounds else {
            return
        }
        let size = icon.size
        iconLayer.frame = NSMakeRect((bounds.width-size.width)/2, (bounds.height-size.height)/2, size.width, size.height)
        iconLayer.contents = icon
    }
    
    internal func animateImage() {
        let animation = CABasicAnimation(keyPath: "contents")
        animation.toValue = isOn ? activeIcon : inactiveIcon
        animation.fromValue = iconLayer.contents
        animation.duration = 0.5
        animation.isRemovedOnCompletion = false
        animation.fillMode = .forwards
        iconLayer.add(animation, forKey: "contents")
        iconLayer.setValue(animation.toValue, forKey: "contents")
    }
    
    internal func setupLayers() {
        rootLayer = CALayer()
        layer = rootLayer
        wantsLayer = true
        layer?.masksToBounds = false
        backgroundLayer = CALayer()
        backgroundLayer!.autoresizingMask = [.layerWidthSizable, .layerHeightSizable]
        backgroundLayer!.bounds = rootLayer!.bounds
        backgroundLayer!.anchorPoint = CGPoint(x: 0.0, y: 0.0)
        backgroundLayer!.borderWidth = kBorderLineWidth
        backgroundLayer?.masksToBounds = false
        
        rootLayer!.addSublayer(backgroundLayer!)
        
        knobLayer = CALayer()
        knobLayer!.frame = rectForKnob()
        knobLayer!.autoresizingMask = CAAutoresizingMask.layerHeightSizable
        knobLayer!.backgroundColor = knobBackgroundColor.cgColor
        knobLayer!.shadowColor = NSColor.black.cgColor
        knobLayer!.shadowOffset = CGSize(width:0.0, height:-6.5)
        knobLayer!.shadowRadius = 2.5
        knobLayer!.shadowOpacity = 0.15
        knobLayer?.masksToBounds = false
        rootLayer!.addSublayer(knobLayer!)
        
        knobInsideLayer = CALayer()
        knobInsideLayer!.frame = knobLayer!.bounds
        knobInsideLayer!.autoresizingMask = [.layerHeightSizable, .layerWidthSizable]
        
        iconLayer = CALayer()
        knobInsideLayer?.addSublayer(iconLayer)
        
        knobInsideLayer!.shadowColor = NSColor.black.cgColor
        knobInsideLayer!.shadowOffset = CGSize(width:0.0, height:0.0)
        knobInsideLayer!.backgroundColor = NSColor.white.cgColor
        knobInsideLayer!.shadowRadius = 1.0
        knobInsideLayer!.shadowOpacity = 0.35
        knobLayer!.addSublayer(knobInsideLayer!)
        
        reloadLayerSize()
        reloadLayer()
    }
    
    internal func reloadLayerSize() {
        CATransaction.begin()
        CATransaction.setDisableActions(true)
        knobLayer!.frame = rectForKnob()
        knobInsideLayer!.frame = knobLayer!.bounds
        backgroundLayer!.cornerRadius = backgroundLayer!.bounds.size.height / 2.0
        knobLayer!.cornerRadius = knobLayer!.bounds.size.height / 2.0
        knobInsideLayer!.cornerRadius = knobLayer!.bounds.size.height / 2.0
        CATransaction.commit()
    }
    
    public func reloadLayerAnimated(animated: Bool) {
        guard let backgroundLayer = backgroundLayer else {
            return
        }
        CATransaction.begin()
        CATransaction.setAnimationDuration(animated ? animationDuration : 0)
        
        if (hasDragged && isDraggingTowardsOn) || (!hasDragged && isOn) {
            backgroundLayer.borderColor = tintColor.cgColor
            backgroundLayer.backgroundColor = tintColor.cgColor
        }
        else {
            backgroundLayer.borderColor = disabledBorderColor.cgColor
            backgroundLayer.backgroundColor = disabledBackgroundColor.cgColor
        }
        
        knobLayer?.shadowColor = isEnabled ? NSColor.black.cgColor : NSColor.clear.cgColor
        rootLayer!.opacity = isEnabled ? kEnabledOpacity : kDisabledOpacity
        
        if hasDragged {
            let function = CAMediaTimingFunction(controlPoints: 0.25, 1.5, 0.5, 1)
            CATransaction.setAnimationTimingFunction(function)
        }
        
        knobLayer!.frame = rectForKnob()
        knobInsideLayer!.frame = knobLayer!.bounds
        
        CATransaction.commit()
        animateImage()
    }
    
    public func reloadLayer() {
        reloadLayerAnimated(animated: true)
    }
    
    internal func knobHeightForSize(_ size:NSSize) -> CGFloat {
        return size.height - kBorderLineWidth * 2.0;
    }
    
    internal func rectForKnob()->CGRect {
        let height = knobHeightForSize(backgroundLayer!.bounds.size)
        var width = 0.0
        
        let bounds: CGRect = backgroundLayer!.bounds
        if ignoreRatio {
            width = Double(height)
        } else {
            if  isActive {
                width = Double((bounds.width - 2.0 * kBorderLineWidth) / kGoldenRatio)
            }
            else {
                width = Double((bounds.width - 2.0 * kBorderLineWidth) / kDecreasedGoldenRatio)
            }
        }
        
        var x:CGFloat = 0
        if (!hasDragged && !isOn) || (hasDragged && !isDraggingTowardsOn) {
            x = kBorderLineWidth
        }
        else {
            x = bounds.width - CGFloat(width) - kBorderLineWidth
        }
        
        return CGRect(x: x, y: kBorderLineWidth, width: CGFloat(width), height: height)
    }
    
    override public func setFrameSize(_ newSize: NSSize) {
        super.setFrameSize(newSize)
        
        reloadLayerSize()
    }
    
    override public func acceptsFirstMouse(for theEvent: NSEvent!) -> Bool {
        return true
    }
    
    
    override public func mouseDown(with theEvent: NSEvent) {
        if !isEnabled || lockInteraction {
            return
        }
        isActive = true
        reloadLayer()
    }
    
    override public func mouseDragged(with theEvent: NSEvent) {
        dragEvents += 1
        guard dragEvents > 3 else {
            return
        }
        dragEvents = 0
        if !isEnabled || lockInteraction {
            return
        }
        hasDragged = true
        
        let draggingPoint = convert(theEvent.locationInWindow, from: nil)
        isDraggingTowardsOn = draggingPoint.x > bounds.width  / 2.0
        reloadLayer()
    }
    
    override public func mouseUp(with theEvent: NSEvent) {
        dragEvents = 0
        if !isEnabled || lockInteraction  {
            return
        }
        
        var on = isOn
        isActive = false
        if hasDragged {
            on = isDraggingTowardsOn
        }
        else {
            on = !isOn
        }
        
        if isOn != on {
            isOn = on
            if action != nil {
                NSApp.sendAction(action!, to: target, from: self)
            }
        } else {
            isOn = on
        }
        
        hasDragged = false
        isDraggingTowardsOn = false
        
        reloadLayer()
    }
    
}

