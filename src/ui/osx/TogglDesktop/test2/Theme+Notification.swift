//
//  Theme+Notification.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 1/16/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

extension Notification.Name {
    static let EffectiveAppearanceChanged = Notification.Name("EffectiveAppearanceChanged")
}

extension NSNotification {
    @objc static let EffectiveAppearanceChanged: String = Notification.Name.EffectiveAppearanceChanged.rawValue
}

extension NSWindow {

    @objc func observerEffectiveAppearanceNotification() -> NSKeyValueObservation {
        return observe(\.effectiveAppearance) {[weak self] (_, _) in
            guard let _ = self else { return }
            NotificationCenter.default.post(name: .EffectiveAppearanceChanged, object: nil)
        }
    }
}
