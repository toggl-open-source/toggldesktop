//
//  OnboardingService+Objc.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 4/1/20.
//  Copyright © 2020 Alari. All rights reserved.
//

import Foundation

final class OnboardingServiceObjc: NSObject {

    @objc class func present(hint: OnboardingHint, atView: NSView) {
        OnboardingService.shared.present(hint: hint, view: atView)
    }
}
