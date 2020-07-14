//
//  TouchBarService+Name.swift
//  TogglTrack
//
//  Created by Nghia Tran on 10/8/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

@available(OSX 10.12.2, *)
extension NSTouchBar.CustomizationIdentifier {
    static let mainTouchBar = NSTouchBar.CustomizationIdentifier("com.toggl.toggltrack.maintouchbar")
    static let idleNotificationTouchBar = NSTouchBar.CustomizationIdentifier("com.toggl.toggltrack.idleNotificationTouchBar")
    static let loginSignUpTouchBar = NSTouchBar.CustomizationIdentifier("com.toggl.toggltrack.loginSignUpTouchBar")
}

@available(OSX 10.12.2, *)
extension NSTouchBarItem.Identifier {

    static let timeEntryItem = NSTouchBarItem.Identifier("com.toggl.toggltrack.timeentrytouchbar.timeentryitems")
    static let runningTimeEntry = NSTouchBarItem.Identifier("com.toggl.toggltrack.timeentrytouchbar.runningtimeentry")
    static let startStopItem = NSTouchBarItem.Identifier("com.toggl.toggltrack.timeentrytouchbar.startstopbutton")

    static let discardIdleItem = NSTouchBarItem.Identifier("com.toggl.toggltrack.timeentrytouchbar.discardIdleItem")
    static let discardIdleAndContinueItem = NSTouchBarItem.Identifier("com.toggl.toggltrack.timeentrytouchbar.discardIdleAndContinueItem")
    static let keepIdleTimeItem = NSTouchBarItem.Identifier("com.toggl.toggltrack.timeentrytouchbar.keepIdleTimeItem")
    static let addIdleTimeItem = NSTouchBarItem.Identifier("com.toggl.toggltrack.timeentrytouchbar.addIdleTimeItem")

    static let loginItem = NSTouchBarItem.Identifier("com.toggl.toggltrack.timeentrytouchbar.loginItem")
    static let loginGoogleItem = NSTouchBarItem.Identifier("com.toggl.toggltrack.timeentrytouchbar.loginGoogleItem")
    static let loginAppleItem = NSTouchBarItem.Identifier("com.toggl.toggltrack.timeentrytouchbar.loginAppleItem")
    static let signUpItem = NSTouchBarItem.Identifier("com.toggl.toggltrack.timeentrytouchbar.signUpItem")
    static let signUpGoogleItem = NSTouchBarItem.Identifier("com.toggl.toggltrack.timeentrytouchbar.signUpGoogleItem")
    static let signUpAppleItem = NSTouchBarItem.Identifier("com.toggl.toggltrack.timeentrytouchbar.signUpAppleItem")
}
