//
//  OnboardingService+Objc.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 4/1/20.
//  Copyright © 2020 Alari. All rights reserved.
//

import Foundation

@objc final class OnboardingServiceObjc: NSObject {

    @objc class func hintFrom(value: Int) -> OnboardingHint {
        return OnboardingHint(rawValue: value) ?? .none
    }

    @objc class func present(hintValue: Int, atView: NSView) {
        guard let hint = OnboardingHint(rawValue: hintValue) else { return }
        OnboardingService.shared.present(hint: hint, view: atView)
    }
}
