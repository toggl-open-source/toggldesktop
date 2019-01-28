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
        self.wantsLayer = true
        self.shadow = NSShadow()
        if #available(OSX 10.13, *) {
            self.layer?.backgroundColor = NSColor(named: NSColor.Name("error-background-color"))?.cgColor
        } else {
            // Fallback on earlier versions
            self.layer?.backgroundColor = NSColor.white.cgColor
        }
        self.layer?.cornerRadius = 8
        self.layer?.borderWidth = 1;
        self.layer?.borderColor = NSColor(white: 0.0, alpha: 0.1).cgColor
        self.layer?.shadowOpacity = 0.13
        self.layer?.shadowColor = NSColor.black.cgColor
        self.layer?.shadowOffset = NSMakeSize(0, -2)
        self.layer?.shadowRadius = 6
    }

    // MARK: Func

    @objc func update(error: String) {
        self.errorLabel.stringValue = error
    }

    @IBAction private func closeOnTap(_ sender: Any) {
        delegate?.floatingErrorShouldHide()
    }
}
