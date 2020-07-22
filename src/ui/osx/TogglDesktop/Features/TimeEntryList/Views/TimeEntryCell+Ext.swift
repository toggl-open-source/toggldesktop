//
//  TimeEntryCell+Ext.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 2/26/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

extension TimeEntryCell {

    @objc enum Position: Int {
        case top
        case all
        case bottom
        case bottomRight

        var roundedCorners: Corners {
            switch self {
            case .top:
                return [.topLeft, .topRight]
            case .bottom:
                return [.bottomLeft, .bottomRight]
            case .all:
                return [.topLeft, .topRight, .bottomLeft, .bottomRight]
            case .bottomRight:
                return .bottomRight
            }
        }
    }

    @objc func mask(for position: Position, rect: NSRect, cornerRadius: CGFloat) -> CALayer {
        let mask = CAShapeLayer()
        let corners = position.roundedCorners
        mask.path = NSBezierPath(rect: rect,
                                 roundedCorners: corners,
                                 cornerRadius: cornerRadius).cgPath
        return mask
    }
}
