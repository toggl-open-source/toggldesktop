//
//  TimelineLineView.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 2/12/20.
//  Copyright © 2020 Alari. All rights reserved.
//

import Foundation

final class TimelineLineView: NSView {

    override var isFlipped: Bool {
        return true
    }

    private lazy var color: NSColor = {
        if #available(OSX 10.13, *) {
            return NSColor(named: NSColor.Name("timeline-divider-color"))!
        } else {
            return ConvertHexColor.hexCode(toNSColor: "#e8e8e8")
        }
    }()

    override func draw(_ dirtyRect: NSRect) {
        super.draw(dirtyRect)

        // Drawing code here.
        let path = NSBezierPath()
        path.move(to: NSPoint(x: NSMinX(bounds), y: NSMinY(bounds)))
        path.line(to: NSPoint(x: NSMaxX(bounds), y: NSMaxY(bounds)))
        path.lineWidth = 1

        // Dashed line
        let dashes: [CGFloat] = [4.0, 8.0]
        path.setLineDash(dashes, count: dashes.count, phase: 0.0)
        path.lineCapStyle = .butt

        // Draw
        color.setStroke()
        path.stroke()
    }
}
