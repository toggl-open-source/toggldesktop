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

    static let shared = SystemPermissionManager()

    // MARK: Public

    func isGranted(_ permission: Permission) -> Bool {
        switch permission {
        case .screenRecording:
            return canRecordScreen()
        }
    }

    func grant(_ permission: Permission) {
        switch permission {
        case .screenRecording:
            // Trigger
            break
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
}

class CoreGraphicsApis {
    static func windows() -> [NSDictionary]? {
        return (CGWindowListCopyWindowInfo([.optionAll], kCGNullWindowID) as? [NSDictionary])
    }

    static func value<T>(_ cgWindow: NSDictionary, _ key: CFString, _ fallback: T) -> T {
        return cgWindow[key] as? T ?? fallback
    }

    static func image(_ windowNumber: CGWindowID) -> CGImage? {
        return CGWindowListCreateImage(.null, .optionIncludingWindow, windowNumber, [.boundsIgnoreFraming])
    }
}
