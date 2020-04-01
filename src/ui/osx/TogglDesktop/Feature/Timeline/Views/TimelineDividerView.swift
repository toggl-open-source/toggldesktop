//
//  TimelineDividerView.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 6/27/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

final class TimelineDividerView: NSView {

    enum Mode {
        case line
        case dash
    }

    override var isFlipped: Bool {
        return true
    }
    
    var mode: Mode = .line
    private lazy var color: NSColor = {
        if #available(OSX 10.13, *) {
            return NSColor(named: NSColor.Name("timeline-divider-color"))!
        } else {
            return ConvertHexColor.hexCode(toNSColor: "#e8e8e8")
        }
    }()

    // MARK: Public

    func draw(for section:TimelineData.Section) {
        switch section {
        case .timeLabel:
            mode = .line
        case .timeEntry:
            mode = .dash
        default:
            mode = .line
        }
    }

    override func draw(_ dirtyRect: NSRect) {
        super.draw(dirtyRect)

        // Drawing code here.
        let path = NSBezierPath()
        path.move(to: NSPoint(x: NSMinX(bounds), y: NSMinY(bounds)))
        path.line(to: NSPoint(x: NSMaxX(bounds), y: NSMaxY(bounds)))
        path.lineWidth = 1

        // Dashed line
        if mode == .dash {
            let dashes: [CGFloat] = [4.0, 8.0]
            path.setLineDash(dashes, count: dashes.count, phase: 0.0)
            path.lineCapStyle = .butt
        }

        // Draw
        color.setStroke()
        path.stroke()
    }
}
