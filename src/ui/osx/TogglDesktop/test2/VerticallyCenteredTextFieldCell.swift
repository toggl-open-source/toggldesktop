//
//  VerticallyCenteredTextFieldCell.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 3/25/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

@IBDesignable
final class VerticallyCenteredTextFieldCell: NSTextFieldCell {

    @IBInspectable var focusRingCornerRadius: CGFloat = 0
    @IBInspectable var leftPadding: CGFloat = 10.0
    @IBInspectable var rightPadding: CGFloat = 0
    private var isEditingOrSelecting = false

    override func drawingRect(forBounds theRect: NSRect) -> NSRect {
        var newRect = super.drawingRect(forBounds: theRect)

        // Padding
        newRect.origin.x += leftPadding
        newRect.size.width -= leftPadding
        newRect.size.width -= rightPadding
        
        // When the text field is being edited or selected, we have to turn off the magic because it screws up
        // the configuration of the field editor.  We sneak around this by intercepting selectWithFrame and editWithFrame and sneaking a
        // reduced, centered rect in at the last minute.
        if !isEditingOrSelecting {
            // Get our ideal size for current text
            let textSize = self.cellSize(forBounds: theRect)

            //Center in the proposed rect
            let heightDelta = newRect.size.height - textSize.height
            if heightDelta > 0 {
                newRect.size.height -= heightDelta
                newRect.origin.y += heightDelta/2
            }
        }

        return newRect
    }

    override func select(withFrame rect: NSRect,
                         in controlView: NSView,
                         editor textObj: NSText,
                         delegate: Any?,
                         start selStart: Int,
                         length selLength: Int) {
        let newRect = self.drawingRect(forBounds: rect)
        isEditingOrSelecting = true;
        super.select(withFrame: newRect, in: controlView, editor: textObj, delegate: delegate, start: selStart, length: selLength)
        isEditingOrSelecting = false;
    }

    override func edit(withFrame rect: NSRect,
                       in controlView: NSView,
                       editor textObj: NSText,
                       delegate: Any?,
                       event: NSEvent?) {
        let newRect = self.drawingRect(forBounds: rect)
        isEditingOrSelecting = true;
        super.edit(withFrame: newRect, in: controlView, editor: textObj, delegate: delegate, event: event)
        isEditingOrSelecting = false
    }

    override func drawFocusRingMask(withFrame cellFrame: NSRect, in controlView: NSView) {

        // Draw default
        guard focusRingCornerRadius > 0.0 else {
            super.drawFocusRingMask(withFrame: cellFrame, in: controlView)
            return
        }

        // Custome
        // Make forcus ring frame fit with cell size
        let newFrame = cellFrame.insetBy(dx: 2, dy: 1)
        let path = NSBezierPath(roundedRect: newFrame, xRadius: focusRingCornerRadius, yRadius: focusRingCornerRadius)
        path.fill()
    }
}

@IBDesignable
final class VerticallyCenteredButtonCell: NSButtonCell {

    @IBInspectable var focusRingCornerRadius: CGFloat = 0
    @IBInspectable var leftPadding: CGFloat = 0

    override func drawingRect(forBounds theRect: NSRect) -> NSRect {
        var newRect = super.drawingRect(forBounds: theRect)

        // Padding
        newRect.origin.x += leftPadding
        newRect.size.width -= leftPadding

        return newRect
    }

    override func drawFocusRingMask(withFrame cellFrame: NSRect, in controlView: NSView) {

        // Draw default
        guard focusRingCornerRadius > 0.0 else {
            super.drawFocusRingMask(withFrame: cellFrame, in: controlView)
            return
        }

        // Custome
        // Make forcus ring frame fit with cell size
        let newFrame = cellFrame.insetBy(dx: 2, dy: 1)
        let path = NSBezierPath(roundedRect: newFrame, xRadius: focusRingCornerRadius, yRadius: focusRingCornerRadius)
        path.fill()
    }
}
