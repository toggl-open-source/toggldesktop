//
//  OnboardingPayload.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 4/1/20.
//  Copyright © 2020 Alari. All rights reserved.
//

import Foundation

/// Represent type of each onboarding
@objc enum OnboardingHint: Int, CustomDebugStringConvertible {
    case newUser = 0
    case oldUser
    case manualMode
    case timelineTab
    case editTimeEntry
    case timelineTimeEntry
    case timelineView
    case timelineActivity
    case recordActivity // Use TimelineActivityRecorderViewController instead
    case textShortcuts

    var debugDescription: String {
        switch self {
        case .editTimeEntry: return "editTimeEntry"
        case .manualMode: return "manualMode"
        case .newUser: return "newUser"
        case .oldUser: return "oldUser"
        case .recordActivity: return "recordActivity"
        case .timelineActivity: return "timelineActivity"
        case .timelineTab: return "timelineTab"
        case .timelineTimeEntry: return "timelineTimeEntry"
        case .timelineView: return "timelineView"
        case .textShortcuts: return "shortcuts"
        }
    }
}

/// Represent the Tab that the onboarding view will show
@objc enum OnboardingPresentViewTab: Int {
    case timeEntry
    case timeline
}

/// Represent all info that the Onboarding need
struct OnboardingPayload {

    // MARK: Variable

    let hint: OnboardingHint
    let view: NSView
    var positioningRect: (OnboardingHint, NSRect) -> NSRect = { _, rect in rect}

    /// Determine the position of the Popover arrow
    var preferEdges: NSRectEdge {
        switch hint {
        case .newUser,
             .oldUser,
             .editTimeEntry,
             .recordActivity,
             .textShortcuts:
            return .minY
        case .timelineTimeEntry,
              .timelineTab,
             .timelineView:
            return .maxY
        case .timelineActivity:
            return .minX
        default: // Fill more later
            return .maxX
        }
    }

    var fadesBackground: Bool {
        switch hint {
        case .textShortcuts:
            return false
        default:
            return true
        }
    }

    var title: NSAttributedString {
        switch hint {
        case .editTimeEntry:
            return NSAttributedString(string: "Click on Time Entry to edit it!")
        case .manualMode:
            return NSAttributedString(string: "It’s also possible to add Time Entries manually!\n\nChange to manual mode there...")
        case .newUser:
            return NSAttributedString(string: "Describe your task and start tracking!")
        case .oldUser:
            return NSAttributedString(string: "Start typing to access your tasks")
        case .recordActivity:
            return NSAttributedString(string: "Having trouble recalling what you were working on?")
        case .timelineActivity:
            return NSAttributedString(string: "Your recorded activity will be shown right here")
        case .timelineTab:
            return NSAttributedString(string: "See your Time Entries on Timeline!")
        case .timelineTimeEntry:
            return NSAttributedString(string: "Add and edit Time Entries in this area")
        case .timelineView:
            return NSAttributedString(string: "See all your Time Entries visualised in chronological order! ")
        case .textShortcuts:
            return shortcutsAttributedTitle
        }
    }

    // MARK: Init

    init(hint: OnboardingHint, view: NSView) {
        self.hint = hint
        self.view = view
    }

    // MARK: Private

    private var shortcutsAttributedTitle: NSAttributedString {
        let titleAttributes: [NSAttributedString.Key: Any] = [
            .font: NSFont.systemFont(ofSize: 12, weight: .semibold)
        ]
        let bodyAttributes: [NSAttributedString.Key: Any] = [
            .font: NSFont.systemFont(ofSize: 12)
        ]
        let shortcutAttributes: [NSAttributedString.Key: Any] = [
            .font: NSFont.systemFont(ofSize: 16, weight: .semibold)
        ]

        let string = NSMutableAttributedString(
            string: "We’ve made it even simpler to add projects and tags!\n",
            attributes: titleAttributes
        )
        string.append(
            NSAttributedString(
                string: "Click on the icon or type ",
                attributes: bodyAttributes
            )
        )
        string.append(
            NSAttributedString(
                string: "@",
                attributes: shortcutAttributes
            )
        )
        string.append(
            NSAttributedString(
                string: " to add projects or\n",
                attributes: bodyAttributes
            )
        )
        string.append(
            NSAttributedString(
                string: "#",
                attributes: shortcutAttributes
            )
        )
        string.append(
            NSAttributedString(
                string: " to add tags.",
                attributes: bodyAttributes
            )
        )
        return string
    }
}
