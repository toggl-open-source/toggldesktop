//
//  TimelineDividerView.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 6/27/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

/// A Supplementary view that draw the Seperator of each Timeline Section
/// It's a dashed line that dividing Time Lable, Time Entry and Activity sections
final class TimelineDividerView: NSView {

    enum Mode {
        case line
        case dash
    }

    override var isFlipped: Bool {
        return true
    }

    var mode: Mode = .line
    private let color: NSColor = Color.timelineDivider.color

    // MARK: Public

    func draw(for section: TimelineData.Section) {
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
        path.move(to: NSPoint(x: bounds.minX, y: bounds.minY))
        path.line(to: NSPoint(x: bounds.maxX, y: bounds.maxY))
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
