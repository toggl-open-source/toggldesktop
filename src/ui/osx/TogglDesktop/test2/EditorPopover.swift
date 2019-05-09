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

    private var isDarkMode = false {
        didSet {
            if isDarkMode {
                if #available(OSX 10.14, *) {
                    appearance = NSAppearance(named: NSAppearance.Name.darkAqua)
                } else {
                    appearance = NSAppearance(named: .aqua)
                }
            } else {
                appearance = NSAppearance(named: .aqua)
            }
        }
    }

    override var contentViewController: NSViewController? {
        didSet {
            DarkMode.onChange = {[weak self] isDarkmode in
                self?.isDarkMode = isDarkmode
            }
            if let isDarkMode = contentViewController?.view.isDarkMode, isDarkMode == true {
                self.isDarkMode = true
            } else {
                self.isDarkMode = false
            }
        }
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
