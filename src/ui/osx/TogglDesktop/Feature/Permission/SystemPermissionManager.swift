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

        var urlScheme: String {
            switch self {
            case .screenRecording:
                return "x-apple.systempreferences:com.apple.preference.security?Privacy_ScreenCapture"
            }
        }
    }

    private struct Constants {
        static let ScreenRecordingPermissionGranted = "ScreenRecordingPermissionGranted"
    }

    static let shared = SystemPermissionManager()

    // MARK: Public

    func isGranted(_ permission: Permission) -> Bool {
        switch permission {
        case .screenRecording:
            if #available(OSX 10.15, *) {
                if isAlreadyRequestSystemPermission(permission) {
                    return canRecordScreen()
                }
                return false
            }
            return true // macOS < 10.15 are granted by default
        }
    }

    func grant(_ permission: Permission) {
        // Skip if it's already granted
        guard !isGranted(permission) else { return }

        switch permission {
        case .screenRecording:
            if #available(OSX 10.15, *) {

                // Show alert to instruct the user to manually grant the permission
                if isAlreadyRequestSystemPermission(permission) {
                    presentScreenRecordingAlert {
                        tryOpeningSystemPreference(for: permission)
                    }
                } else {
                    // Trigger the system alert once
                    triggerScreenRecordingPermissionAlert()
                }
            }
        }
    }
}

// MARK: Private

extension SystemPermissionManager {

    private func presentScreenRecordingAlert(_ complete: () -> Void) {
        let alert = NSAlert()
        alert.messageText = "Screen Recording permission not granted!"
        alert.informativeText = "To get the focused application window name properly for the Timeline, TogglDesktop needs to be granted the Screen Recording permission in Security & Privacy in System Preferences .\n\nPlease open System Preferences -> Security & Privacy -> Privacy Tab -> Select Screen Recording and enable TogglDesktop app."
        alert.alertStyle = .warning
        alert.addButton(withTitle: "Open")
        alert.addButton(withTitle: "Later")

        let result = alert.runModal()
        if result == .alertFirstButtonReturn {
            complete()
        }
    }

    private func isAlreadyRequestSystemPermission(_ permission: Permission) -> Bool {
        switch permission {
        case .screenRecording:
            return UserDefaults.standard.bool(forKey: Constants.ScreenRecordingPermissionGranted)
        }
    }

    private func tryOpeningSystemPreference(for permission: Permission) {

        // Open the Screen Recording tab
        // https://stackoverflow.com/a/48139877/3127477
        if let url = URL(string: permission.urlScheme) {
            NSWorkspace.shared.open(url)
            return
        }

        // Old-fashioned way
        // https://github.com/cho45/KeyCast/blob/master/KeyCast/Accessibility.swift
        let url = URL(fileURLWithPath: "/System/Library/PreferencePanes/Security.prefPane")
        NSWorkspace.shared.open(url)
    }
}

// MARK: Screen Recording

extension SystemPermissionManager {

    // CGWindowListCopyWindowInfo is not reliable because some windows has the name even the permission is OFF
    // Reliable way to check whether the permission is enable or not
    // However it would trigger the system alert too
    private func canRecordScreen() -> Bool {
        let mainDisplay    = CGMainDisplayID()
        let displayBounds  = CGDisplayBounds(mainDisplay)
        let recordingQueue = DispatchQueue.global(qos: .background)
        let displayStreamProps : [CFString : Any] = [
            CGDisplayStream.preserveAspectRatio: kCFBooleanTrue,
            CGDisplayStream.showCursor:          kCFBooleanTrue,
            CGDisplayStream.minimumFrameTime:    60,
        ]

        let displayStream = CGDisplayStream(
            dispatchQueueDisplay: mainDisplay,
            outputWidth:          Int(displayBounds.width),
            outputHeight:         Int(displayBounds.height),
            pixelFormat:          Int32(kCVPixelFormatType_32BGRA),
            properties:           displayStreamProps as CFDictionary,
            queue:                recordingQueue,
            handler: { status, displayTime, frameSurface, updateRef in
                print("is only called once")
        }
        )
        return displayStream != nil
    }

    private func triggerScreenRecordingPermissionAlert() {
        guard let window = CoreGraphicsApis.windows()?.last else { return }
        let windowNumber = (window[kCGWindowNumber as String] as? CGWindowID) ?? UInt32(0)

        // There is no func to explicit trigger the permission alert
        // Have to execute CGWindowListCreateImage -> OS will present the alert once
        CoreGraphicsApis.image(windowNumber)

        // Save
        UserDefaults.standard.set(true, forKey: Constants.ScreenRecordingPermissionGranted)
        UserDefaults.standard.synchronize()
    }
}

// MARK: Helper class

private final class CoreGraphicsApis {

    static func windows() -> [[String: AnyObject]]? {
        return (CGWindowListCopyWindowInfo([.optionOnScreenOnly], kCGNullWindowID) as? [[String: AnyObject]])
    }

    static func value<T>(_ cgWindow: NSDictionary, _ key: CFString, _ fallback: T) -> T {
        return cgWindow[key] as? T ?? fallback
    }

    @discardableResult
    static func image(_ windowNumber: CGWindowID) -> CGImage? {
        return CGWindowListCreateImage(.null, .optionIncludingWindow, windowNumber, [.boundsIgnoreFraming])
    }
}
