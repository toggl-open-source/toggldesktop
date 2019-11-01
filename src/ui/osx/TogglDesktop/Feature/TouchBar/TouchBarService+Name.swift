//
//  TouchBarService+Name.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 10/8/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

@available(OSX 10.12.2, *)
extension NSTouchBar.CustomizationIdentifier {
    static let mainTouchBar = NSTouchBar.CustomizationIdentifier("com.toggl.toggldesktop.maintouchbar")
}

@available(OSX 10.12.2, *)
extension NSTouchBarItem.Identifier {

    static let timeEntryItem = NSTouchBarItem.Identifier("com.toggl.toggldesktop.timeentrytouchbar.timeentryitems")
    static let runningTimeEntry = NSTouchBarItem.Identifier("com.toggl.toggldesktop.timeentrytouchbar.runningtimeentry")
    static let startStopItem = NSTouchBarItem.Identifier("com.toggl.toggldesktop.timeentrytouchbar.startstopbutton")
    static let cancelItem = NSTouchBarItem.Identifier("com.toggl.toggldesktop.timeentrytouchbar.cancelactionbutton")
    static let deleteItem = NSTouchBarItem.Identifier("com.toggl.toggldesktop.timeentrytouchbar.deletetimeentrybutton")
}
