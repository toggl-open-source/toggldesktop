//
//  CursorButton.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 3/26/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

class CursorButton: NSButton {

    // MARK: - Variable

    var cursor: NSCursor? {
        didSet {
            resetCursorRects()
        }
    }

    override func resetCursorRects() {
        if let cursor = cursor {
            addCursorRect(bounds, cursor: cursor)
        } else {
            super.resetCursorRects()
        }
    }
}
