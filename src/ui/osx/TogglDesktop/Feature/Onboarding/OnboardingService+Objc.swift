//
//  OnboardingService+Objc.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 4/1/20.
//  Copyright © 2020 Alari. All rights reserved.
//

import Foundation

final class OnboardingServiceObjc: NSObject {

    @objc class func present(hintValue: Int, atView: NSView) {
        guard let hint = OnboardingHint(rawValue: hintValue) else { return }
        OnboardingService.shared.present(hint: hint, view: atView)
    }
}
