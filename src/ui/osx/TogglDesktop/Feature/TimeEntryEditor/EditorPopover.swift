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

    
    override func viewDidMoveToWindow() {
        guard let frameView = window?.contentView?.superview else { return }

        let backgroundView = PopoverMainView(frame: frameView.bounds)
        backgroundView.autoresizingMask = [.width, .height]
        frameView.addSubview(backgroundView, positioned: .below, relativeTo: frameView)
    }
}

class PopoverMainView:NSView {
    override func draw(_ dirtyRect: NSRect) {
        Color(named: "MyColor")!.set()
        self.bounds.fill()
    }
}
