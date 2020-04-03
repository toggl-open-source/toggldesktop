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

    private lazy var maskLayer = CAShapeLayer()
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

    func drawMask(at frame: CGRect) {
        let path = NSBezierPath(rect: frame)
        path.append(NSBezierPath(rect: bounds))
        maskLayer.path = path.cgPath
        layer?.mask = maskLayer
    }
}

// MARK: Private

extension OnboardingBackgroundView {

    private func initCommon() {
        wantsLayer = true
        layer?.backgroundColor = backgroundColor.cgColor
        maskLayer.backgroundColor = NSColor.white.cgColor
        maskLayer.fillRule = .evenOdd // invert mask
    }
}
