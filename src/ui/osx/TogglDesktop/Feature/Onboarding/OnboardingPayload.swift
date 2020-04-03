//
//  OnboardingPayload.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 4/1/20.
//  Copyright © 2020 Alari. All rights reserved.
//

import Foundation

enum OnboardingHint: Int {
    case newUser
    case oldUser
    case manualMode
    case timelineTab
    case editTimeEntry
    case timelineTimeEntry
    case timelineView
    case timelineActivity // Use TimelineActivityRecorderViewController instead
    case recordActivity
}

struct OnboardingPayload {

    // MARK: Variable

    let title: String
    let hint: OnboardingHint

    var preferEdges: NSRectEdge {
        switch hint {
        case .timelineTab:
            return .maxY
        default: // Fill more later
            return .maxX
        }
    }

    // MARK: Init

    init(hint: OnboardingHint) {
        self.hint = hint
        switch hint {
        case .editTimeEntry: self.title = "Click on Time Entry to edit it!"
        case .manualMode: self.title = "It’s also possible to add Time Entries manually!\n\nChange to manual mode there..."
        case .newUser: self.title = "Describe your task and start tracking!"
        case .oldUser: self.title = "Start typing to access your tasks"
        case .recordActivity: self.title = "Your recorded activity will be shown right here"
        case .timelineActivity: self.title = "Having troubles recalling what you were working on?"
        case .timelineTab: self.title = "See your Time Entries on Timeline!"
        case .timelineTimeEntry: self.title = "Add and edit Time Entries in this area"
        case .timelineView: self.title = "See all your Time Entries visualised in chronological order! "
        }
    }
}
