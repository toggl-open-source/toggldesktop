//
//  EditorPopover.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 3/12/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

final class EditorPopover: NoVibrantPopoverView {

    private struct Constants {
        static let FocusTimerNotification = NSNotification.Name(kFocusTimer)
    }

    @objc func prepareViewController() {
        let editor = EditorViewController(nibName: NSNib.Name("EditorViewController"), bundle: nil)
        editor.delegate = self
        contentViewController = editor
    }

    override func close(focusTimer: Bool) {
        super.close(focusTimer: focusTimer)
        if focusTimer {
            NotificationCenter.default.post(name: Constants.FocusTimerNotification, object: nil)
        }
    }

    @objc func setTimeEntry(_ timeEntry: TimeEntryViewItem) {
        if let editor = contentViewController as? EditorViewController {
            editor.timeEntry = timeEntry
        }
    }
}

// MARK: EditorViewControllerDelegate

extension EditorPopover: EditorViewControllerDelegate {

    func editorShouldDismissPopover() {
        close(focusTimer: false)
    }
}
