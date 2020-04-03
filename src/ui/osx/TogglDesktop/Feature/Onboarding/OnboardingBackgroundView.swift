//
//  OnboardingBackgroundView.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 4/3/20.
//  Copyright © 2020 Alari. All rights reserved.
//

import Cocoa

final class OnboardingBackgroundView: NSView {

    // MARK: Variable

    private lazy var maskLayer = CALayer()
    private lazy var backgroundColor: NSColor = {
        if #available(OSX 10.13, *) {
            return NSColor(named: NSColor.Name("onboarding-background-color"))!
        } else {
            return NSColor(calibratedWhite: 0, alpha: 0.5)
        }
    }()

    // MARK: Init

    override init(frame frameRect: NSRect) {
        super.init(frame: frameRect)
        initCommon()
    }

    required init?(coder: NSCoder) {
        super.init(coder: coder)
        initCommon()
    }

    override func awakeFromNib() {
        super.awakeFromNib()
        initCommon()
    }

    // MARK: Public

    func setMaskPosition(at view: NSView) {
        maskLayer.backgroundColor = NSColor.white.cgColor
        maskLayer.frame = CGRect(x: 0, y: 0, width: 100, height: 100)

        if layer?.mask == nil {
            layer?.mask = maskLayer
        }
    }

}

// MARK: Private

extension OnboardingBackgroundView {

    private func initCommon() {
        wantsLayer = true
        layer?.backgroundColor = backgroundColor.cgColor
    }
}
