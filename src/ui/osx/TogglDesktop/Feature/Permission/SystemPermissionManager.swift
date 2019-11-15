//
//  SystemPermissionManager.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 11/15/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

final class SystemPermissionManager {

    enum Permission {
        case screenRecording
    }

    private struct Constants {
        static let ScreenRecordingPermissionGranted = "ScreenRecordingPermissionGranted"
    }

    static let shared = SystemPermissionManager()

    // MARK: Public

    func isGranted(_ permission: Permission) -> Bool {
        switch permission {
        case .screenRecording:
            return canRecordScreen()
        }
    }

    func grant(_ permission: Permission, showInstruction: Bool) {
        switch permission {
        case .screenRecording:
            // Show alert to instruct the user to manually grant the permission
            if isAlreadyPresentPermissionAlert(permission) {

            } else {

            }
        }
    }
}

extension SystemPermissionManager {

    private func buildAlert() {
        let alert = NSAlert()
        alert.messageText = "Screen Recording permission not granted!"
        alert.informativeText = "To get the Windows Name properly for the Timeline, TogglDesktop needs to be granted the Screen Recording permission in Security & Privacy in System Preferences .\n\nPlease open System Preferences -> Security & Privacy -> Privacy Tab -> Select Screen Recording and enable TogglDesktop app."
        alert.alertStyle = .warning
        alert.addButton(withTitle: "Open")
        alert.addButton(withTitle: "Later")

        let result = alert.runModal()
        if result == .alertFirstButtonReturn {
            if let url = URL(string: "x-apple.systempreferences:com.apple.preference.security?Privacy_ScreenCapture") {
                NSWorkspace.shared.open(url)
            }

        }
    }

    private func isAlreadyPresentPermissionAlert(_ permission: Permission) -> Bool {
        switch permission {
        case .screenRecording:
            return UserDefaults.standard.bool(forKey: Constants.ScreenRecordingPermissionGranted)
        }
    }

    private func tryOpeningSystemPreference(for permission: Permission) {
        if let url = URL(string: "x-apple.systempreferences:com.apple.preference.security?Privacy_ScreenCapture") {
            NSWorkspace.shared.open(url)
        } else {
            let url = URL(fileURLWithPath: "/System/Library/PreferencePanes/Security.prefPane")
            NSWorkspace.shared.open(url)
        }
    }
}

// MARK: Screen Recording

extension SystemPermissionManager {

    private func canRecordScreen() -> Bool {
        // If we are able to extract the kCGWindowName from all windows
        // it means user enabled the Screen Recording permission
        guard let windows = CGWindowListCopyWindowInfo([.optionOnScreenOnly], kCGNullWindowID) as? [[String: AnyObject]] else { return false }
        return windows.allSatisfy({ window in
            let windowName = window[kCGWindowName as String] as? String
            return windowName != nil
        })
    }

    private func triggerScreenRecordingPermissionAlert() {
        guard let firstWindow = CoreGraphicsApis.windows()?.last else { return }
        let windowNumber = CoreGraphicsApis.value(firstWindow, kCGWindowNumber, UInt32(0))

        // There is no func to explicit trigger the permission alert
        // Have to execute CGWindowListCreateImage -> OS will present the alert once
        CoreGraphicsApis.image(windowNumber)
    }
}

// MARK: Helper class

private final class CoreGraphicsApis {

    static func windows() -> [NSDictionary]? {
        return (CGWindowListCopyWindowInfo([.optionAll], kCGNullWindowID) as? [NSDictionary])
    }

    static func value<T>(_ cgWindow: NSDictionary, _ key: CFString, _ fallback: T) -> T {
        return cgWindow[key] as? T ?? fallback
    }

    @discardableResult
    static func image(_ windowNumber: CGWindowID) -> CGImage? {
        return CGWindowListCreateImage(.null, .optionIncludingWindow, windowNumber, [.boundsIgnoreFraming])
    }
}
