//
//  CustomFocusRingButton.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 4/10/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

final class CustomFocusRingButton: CursorButton {

    override func drawFocusRingMask() {
        let rect = self.bounds
        rect.fill()
    }
}
