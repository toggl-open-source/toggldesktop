//
//  OnboardingService+Objc.swift
//  TogglTrack
//
//  Created by Nghia Tran on 4/1/20.
//  Copyright © 2020 Alari. All rights reserved.
//

import Foundation

/// Expose all Onboarding swift class to Objc world
/// We dont' directly expose OnboardingService class
@objc final class OnboardingServiceObjc: NSObject {

    /// Determine if there is any onboarding is shown
    /// - Returns: Onboarding is shown
    @objc class func isOnboardingShown() -> Bool {
        return OnboardingService.shared.isShown
    }

    /// Helper func to handle the Onboarding Notification from MainWindowController.m
    /// It will initialize the Onboarding Hint and present on view if need
    /// - Parameters:
    ///   - noti: The onboarding notification from the library
    ///   - atView: A block to get the view that onboarding sticks on
    ///   - switchTo: A block to ask the view controller switch to the certain tab
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
        print("✅ Present onboarding hint = \(hint.debuggingName)")
        let payload = OnboardingPayload(hint: hint, view: view)
        OnboardingService.shared.present(payload)
    }

    /// Get the parent view that the onboarding should be shown
    /// It could be a Timeline or Time Entry List
    /// - Parameter hint: Onboarding Hint
    /// - Returns: The view that the onboarding should be shown
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
