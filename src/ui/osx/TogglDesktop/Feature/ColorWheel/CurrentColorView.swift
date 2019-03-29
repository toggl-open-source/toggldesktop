//
//  CurrentColorView.swift
//  Color Picker Plus
//
//  Created by Viktor Hundahl Strate on 14/06/2018.
//  Copyright © 2018 Viktor Hundahl Strate. All rights reserved.
//

import Cocoa

@IBDesignable
class CurrentColorView: NSView {
    
    var previousColor: NSColor? {
        didSet {
            needsDisplay = true
        }
    }
    var color: NSColor? {
        didSet {
            if (previousColor == nil) {
                previousColor = color
            }
            needsDisplay = true
        }
    }
    
    override func mouseUp(with event: NSEvent) {
        
        var previousColorRect = convert(bounds, to: window?.contentView)
        previousColorRect.size = NSSize(width: bounds.width, height: bounds.height / 2)
        
        //let previousColorRect = NSRect(x: frame.minX, y: frame.minY, width: frame.width, height: frame.height / 2)
        
        if (previousColorRect.contains(event.locationInWindow)) {
            previousColor = color
        }
    }

    override func draw(_ dirtyRect: NSRect) {
        super.draw(dirtyRect)
        
        let context = NSGraphicsContext.current!.cgContext
        
        guard color != nil else {
            NSColor.black.setFill()
            context.fill(bounds)
            return
        }
        
        // Background for alpha colors
        ColorGraphicsView.drawTransparentGridOverlay(rect: bounds, context: context)
        
        if (previousColor != nil) {
            color!.setFill()
            
            let topRect = NSRect(x: bounds.minX, y: bounds.minY + bounds.height / 2,
                                 width: bounds.width, height: bounds.height / 2)
            let bottomRect = NSRect(x: bounds.minX, y: bounds.minY, width: bounds.width, height: bounds.height / 2)
            
            context.fill(topRect)
            
            previousColor!.setFill()
            context.fill(bottomRect)
        } else {
            color!.setFill()
            //NSRectFill(bounds)
            context.fill(bounds)
        }

        // Drawing code here.
    }
    
}
