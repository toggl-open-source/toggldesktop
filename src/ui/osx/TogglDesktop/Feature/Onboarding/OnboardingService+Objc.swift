//
//  OnboardingService+Objc.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 4/1/20.
//  Copyright © 2020 Alari. All rights reserved.
//

import Foundation

@objc final class OnboardingServiceObjc: NSObject {

    @objc class func isOnboardingShown() -> Bool {
        return OnboardingService.shared.isShown
    }

    @objc class func handleOnboardingNotification(_ noti: Notification, atView: (OnboardingHint) -> NSView?, switchTo: (OnboardingPresentViewTab) -> Void) {
        guard let number = noti.object as? NSNumber else { return }
        guard let hint = OnboardingHint(rawValue: number.intValue) else {
            return
        }
        guard let view = atView(hint) else {
            print("[ERROR] Couldn't find the present view at \(hint)")
            return
        }

        // Switch to correct Tab
        let presentView = self.presentViewTab(for: hint)
        switchTo(presentView)

        // Present
        print("✅ Present onboarding hint = \(hint)")
        let payload = OnboardingPayload(hint: hint, view: view)
        OnboardingService.shared.present(payload)
    }

    private class func presentViewTab(for hint: OnboardingHint) -> OnboardingPresentViewTab {
        switch hint {
        case .editTimeEntry,
             .manualMode,
             .newUser,
             .oldUser:
            return .timeEntry
        case .recordActivity,
             .timelineActivity,
             .timelineTab,
             .timelineTimeEntry,
             .timelineView:
            return .timeline
        }
    }
}
