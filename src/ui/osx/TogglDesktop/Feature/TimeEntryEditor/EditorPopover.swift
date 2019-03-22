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

    override var appearance: NSAppearance? {
        get {
            if let appearance = NSApplication.shared.windows.first?.effectiveAppearance {
                if appearance.name.rawValue.lowercased().contains("dark") {
                    if #available(OSX 10.14, *) {
                        return NSAppearance(named: .darkAqua)
                    } else {
                        return NSAppearance(named: .aqua)
                    }
                }
            }
            return NSAppearance(named: .aqua)
        }
        set {}
    }

    @objc func prepareViewController() {
        let editor = EditorViewController.init(nibName: NSNib.Name("EditorViewController"), bundle: nil)
        contentViewController = editor
    }

    @objc func present(to rect: NSRect, of view: NSView) {
        show(relativeTo: rect, of: view, preferredEdge: .maxX)
    }

    @objc func close(focusTimer: Bool) {

        // Close and notify delegate if need
        performClose(self)

        // Focus on timer bar
        if focusTimer {
            NotificationCenter.default.post(name: Constants.FocusTimerNotification, object: nil)
        }
    }
}
