//
//  OnboardingPayload.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 4/1/20.
//  Copyright © 2020 Alari. All rights reserved.
//

import Foundation

@objc enum OnboardingHint: Int {
    case newUser = 0
    case oldUser
    case manualMode
    case timelineTab
    case editTimeEntry
    case timelineTimeEntry
    case timelineView
    case timelineActivity
    case recordActivity // Use TimelineActivityRecorderViewController instead
}

@objc enum OnboardingPresentViewTab: Int {
    case timeEntry
    case timeline
}

struct OnboardingPayload {

    // MARK: Variable

    let title: String
    let hint: OnboardingHint
    let view: NSView

    var preferEdges: NSRectEdge {
        switch hint {
        case .newUser,
             .oldUser,
             .editTimeEntry,
             .timelineTab,
             .recordActivity:
            return .minY
        case .timelineTimeEntry,
             .timelineView:
            return .maxY
        case .timelineActivity:
            return .minX
        default: // Fill more later
            return .maxX
        }
    }

    var bounds: CGRect {
        switch hint {
        case .timelineTimeEntry:
            var bounds = view.bounds
            bounds.origin.x = 100
            bounds.size.width -= 200
            return bounds
        default:
            return view.bounds
        }
    }

    // MARK: Init

    init(hint: OnboardingHint, view: NSView) {
        self.hint = hint
        self.view = view
        switch hint {
        case .editTimeEntry: self.title = "Click on Time Entry to edit it!"
        case .manualMode: self.title = "It’s also possible to add Time Entries manually!\n\nChange to manual mode there..."
        case .newUser: self.title = "Describe your task and start tracking!"
        case .oldUser: self.title = "Start typing to access your tasks"
        case .recordActivity: self.title = "Having troubles recalling what you were working on?"
        case .timelineActivity: self.title = "Your recorded activity will be shown right here"
        case .timelineTab: self.title = "See your Time Entries on Timeline!"
        case .timelineTimeEntry: self.title = "Add and edit Time Entries in this area"
        case .timelineView: self.title = "See all your Time Entries visualised in chronological order! "
        }
    }
}
