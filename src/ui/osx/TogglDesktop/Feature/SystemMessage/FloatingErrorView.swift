//
//  FloatingErrorView.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 1/28/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

final class FloatingErrorView: NSView {

    // MARK: Outlet

    @IBOutlet private weak var titleLabel: NSTextField!
    @IBOutlet private weak var subtitleLabel: NSTextField!

    // MARK: Variables
    var onClose: (() -> Void)?
    fileprivate lazy var errorColor: NSColor = {
        if #available(OSX 10.13, *) {
            return NSColor(named: NSColor.Name("error-title-color"))!
        } else {
            return ConvertHexColor.hexCode(toNSColor: "#FF3B30")
        }
    }()
    fileprivate lazy var informativeColor: NSColor = {
        if #available(OSX 10.13, *) {
            return NSColor(named: NSColor.Name("green-color"))!
        } else {
            return ConvertHexColor.hexCode(toNSColor: "#28cd41")
        }
    }()

    // MARK: Init

    class func initFromXib() -> FloatingErrorView {
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

    @IBAction private func closeOnTap(_ sender: Any) {
        onClose?()
    }

    func present(_ payload: SystemMessage.Payload) {

        // Apply text and text color
        switch payload.content {
        case .error(let title, let subtitle):

            // Title
            subtitleLabel.isHidden = subtitle == nil
            subtitleLabel.stringValue = subtitle ?? ""
            titleLabel.stringValue = title

            // color
            titleLabel.textColor = errorColor

        case .informative(let info):

            // Title
            subtitleLabel.isHidden = true
            titleLabel.stringValue = info

            // Color
            titleLabel.textColor = informativeColor
        }
    }
}
