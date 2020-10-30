//
//  AutoCompleteWindow.swift
//  TogglDesktop
//
//  Created by Andrew Nester on 29.10.2020.
//  Copyright © 2020 Toggl. All rights reserved.
//

import Foundation

final class DropdownWindow: NSWindow {

    private enum Constants {
        static let topPadding: CGFloat = 5.0
        static let topPaddingLarge: CGFloat = 30.0
    }

    var isSeparateWindow = true

    override var canBecomeMain: Bool { true }
    override var canBecomeKey: Bool { true }

    // MARK: Init

    init(view: NSView) {
        super.init(contentRect: view.bounds,
                   styleMask: .borderless,
                   backing: .buffered,
                   defer: true)
        contentView = view
        hasShadow = true
        backgroundColor = NSColor.clear
        isOpaque = false
        setContentBorderThickness(0, for: NSRectEdge(rawValue: 0)!)
    }

    func layoutFrame(with textField: NSTextField, origin: CGPoint, size: CGSize) {
        guard let window = textField.window else { return }
        var height = size.height

        // Convert
        var location = CGPoint.zero
        let point = textField.superview!.convert(origin, to: nil)
        if #available(OSX 10.12, *) {
            location = window.convertPoint(toScreen: point)
        } else {
            location = window.convertToScreen(NSRect(origin: point, size: size)).origin
        }

        if isSeparateWindow {
            location.y -= Constants.topPadding
        } else {
            location.y -= -Constants.topPaddingLarge
            height += Constants.topPaddingLarge
        }

        setFrame(CGRect(x: 0, y: 0, width: size.width, height: height), display: false)
        setFrameTopLeftPoint(location)
    }

    func cancel() {
        parent?.removeChildWindow(self)
        orderOut(nil)
    }
}
