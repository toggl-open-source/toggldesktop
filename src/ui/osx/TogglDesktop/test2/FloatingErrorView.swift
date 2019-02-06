//
//  FloatingErrorView.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 1/28/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

@objc protocol FloatingErrorViewDelegate: class {

    func floatingErrorShouldHide()
}

final class FloatingErrorView: NSView {

    // MARK: Outlet

    @IBOutlet private weak var errorLabel: NSTextField!

    // MARK: Variables

    @objc weak var delegate: FloatingErrorViewDelegate?

    // MARK: Init

    @objc class func initFromXib() -> FloatingErrorView {
        return FloatingErrorView.xibView()
    }

    override func awakeFromNib() {
        super.awakeFromNib()
        initCommon()
    }

    private func initCommon() {
        wantsLayer = true
        shadow = NSShadow()
        layer?.cornerRadius = 8
        layer?.borderWidth = 1;
        layer?.borderColor = NSColor(white: 0.0, alpha: 0.1).cgColor
        layer?.shadowOpacity = 0.13
        layer?.shadowColor = NSColor.black.cgColor
        layer?.shadowOffset = NSMakeSize(0, -2)
        layer?.shadowRadius = 6
    }

    override func updateLayer() {
        super.updateLayer()

        if #available(OSX 10.13, *) {
            layer?.backgroundColor = NSColor(named: NSColor.Name("error-background-color"))?.cgColor
        } else {
            // Fallback on earlier versions
            layer?.backgroundColor = NSColor.white.cgColor
        }

    }
    // MARK: Func

    @objc func update(error: String) {
        errorLabel.stringValue = error
    }

    @IBAction private func closeOnTap(_ sender: Any) {
        delegate?.floatingErrorShouldHide()
    }
}
