//
//  EditorPopover.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 3/12/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

final class EditorPopover: NoVibrantPopoverView {

    override init() {
        let size = CGSize(width: 274, height: 381)
        let maxSize = CGSize(width: size.width * 3, height: size.height)
        super.init(min: size, max: maxSize)
    }

    required init?(coder: NSCoder) {
        super.init(coder: coder)
    }

    @objc func prepareViewController() {
        let editor = EditorViewController(nibName: NSNib.Name("EditorViewController"), bundle: nil)
        editor.view.appearance = appearance
        let size = DesktopLibraryBridge.shared().getEditorWindowSize()
        editor.view.frame.size = size
        contentViewController = editor
    }

    @objc func setTimeEntry(_ timeEntry: TimeEntryViewItem) {
        if let editor = contentViewController as? EditorViewController {
            editor.timeEntry = timeEntry
        }
    }

    override func popoverDidResize() {
        DesktopLibraryBridge.shared().setEditorWindowSize(contentSize)
    }
}
