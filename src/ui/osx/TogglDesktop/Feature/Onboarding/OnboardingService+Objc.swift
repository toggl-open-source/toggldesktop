//
//  OnboardingService+Objc.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 4/1/20.
//  Copyright © 2020 Alari. All rights reserved.
//

import Foundation

@objc final class OnboardingServiceObjc: NSObject {

    @objc class func handleOnboardingNotification(_ noti: Notification, atView: (OnboardingHint) -> NSView?) {
        guard let number = noti.object as? NSNumber else { return }
        guard let hint = OnboardingHint(rawValue: number.intValue) else {
            return
        }
        guard let view = atView(hint) else {
            print("[ERROR] Couldn't find the present view at \(hint)")
            return
        }

        print("✅ Present onboarding hint = \(hint)")
        OnboardingService.shared.present(hint: hint, view: view)
    }
}
