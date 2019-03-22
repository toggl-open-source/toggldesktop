//
//  NSView+Animation.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 3/4/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

extension NSView {

    func spinClockwise(timeToRotate: Double) {
        guard let layer = layer else { return }
        wantsLayer = true
        layer.position = CGPoint(x: layer.frame.midX, y: layer.frame.midY)
        layer.anchorPoint = CGPoint(x: 0.5, y: 0.5)
        startRotate(angle: CGFloat(-1 * .pi * 2.0), timeToRotate: timeToRotate)
    }

    func startRotate(angle: CGFloat, timeToRotate: Double) {
        let rotateAnimation = CABasicAnimation(keyPath: "transform.rotation")
        rotateAnimation.fromValue = 0.0
        rotateAnimation.toValue = angle
        rotateAnimation.duration = timeToRotate
        rotateAnimation.repeatCount = .infinity

        layer?.add(rotateAnimation, forKey: nil)
    }

    func stopAnimations() {
        layer?.removeAllAnimations()
    }
}
