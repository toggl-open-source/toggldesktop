//
//  NSView+Ext.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 3/4/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

extension NSView {

    func bringSubviewToFront(_ view: NSView) {
        var theView = view
        self.sortSubviews({(viewA,viewB,rawPointer) in
            let view = rawPointer?.load(as: NSView.self)

            switch view {
            case viewA:
                return ComparisonResult.orderedDescending
            case viewB:
                return ComparisonResult.orderedAscending
            default:
                return ComparisonResult.orderedSame
            }
        }, context: &theView)
    }
}
