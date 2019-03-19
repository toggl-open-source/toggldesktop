//
//  TimeEntryCell+Ext.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 2/26/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

extension TimeEntryCell {

    @objc enum Position: Int{
        case top
        case middle
        case bottom
    }

    @objc func mask(for position: Position, rect: NSRect, cornerRadius: CGFloat) -> CALayer {
        let mask = CAShapeLayer()
        mask.path = NSBezierPath(rect: rect, roundedCorners: [.bottomLeft, .bottomRight], cornerRadius: cornerRadius).cgPath
        return mask
    }
}
