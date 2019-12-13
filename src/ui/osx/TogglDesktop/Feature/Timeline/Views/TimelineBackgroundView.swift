//
//  TimelineBackgroundView.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 12/13/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

final class TimelineBackgroundView: NSView {

    override func draw(_ dirtyRect: NSRect) {
        super.draw(dirtyRect)

        // Drawing code here.
        NSColor.blue.setFill()
        dirtyRect.fill()
    }
    
}
