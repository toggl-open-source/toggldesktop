//
//  Color+Adaptive.swift
//  TogglTrack
//
//  Created by Nghia Tran on 5/25/20.
//  Copyright © 2020 Alari. All rights reserved.
//

import Foundation

@objc extension NSColor {

    @objc func getAdaptiveColorForShape() -> NSColor {
        return DesktopLibraryBridge.shared().getAdaptiveColorForShape(from: self)
    }

    @objc func getAdaptiveColorForText() -> NSColor {
        return DesktopLibraryBridge.shared().getAdaptiveColorForText(from: self)
    }
}
