//
//  Project.swift
//  TogglDesktop
//
//  Created by Andrew Nester on 15.10.2020.
//  Copyright © 2020 Toggl. All rights reserved.
//

import Foundation

struct Project {
    let guid: String?
    let name: String
    let colorHex: String
    let taskName: String?
    let clientName: String?

    var color: NSColor {
        ConvertHexColor.hexCode(toNSColor: colorHex)
    }

    var attributedTitle: NSAttributedString {
        ProjectTitleFactory().title(withProject: name, task: taskName, client: clientName, projectColor: color)
    }

    init(guid: String? = nil, name: String, colorHex: String, taskName: String? = nil, clientName: String? = nil) {
        self.guid = guid
        self.name = name
        self.colorHex = colorHex
        self.taskName = taskName
        self.clientName = clientName
    }

    init?(timeEntry: TimeEntryViewItem) {
        guard timeEntry.projectLabel != nil, timeEntry.projectLabel.isEmpty == false else {
            return nil
        }
        self.guid = timeEntry.guid
        self.colorHex = timeEntry.projectColor
        self.name = timeEntry.projectLabel
        self.taskName = timeEntry.taskLabel
        self.clientName = timeEntry.clientLabel
    }
}
