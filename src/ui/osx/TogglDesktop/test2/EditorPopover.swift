//
//  EditorPopover.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 3/12/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

final class EditorPopover: NSPopover {

    private struct Constants {
        static let FocusTimerNotification = NSNotification.Name(kFocusTimer)
    }

    // MARK: Public

    @objc func close(focusTimer: Bool) {

        // Close and notify delegate if need
        performClose(self)

        // Focus on timer bar
        if focusTimer {
            NotificationCenter.default.post(name: Constants.FocusTimerNotification, object: nil)
        }
    }
}
