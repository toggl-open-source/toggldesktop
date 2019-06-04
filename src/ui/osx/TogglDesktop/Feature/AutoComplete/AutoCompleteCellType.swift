//
//  AutoCompleteCellType.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 3/8/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

@objc enum AutoCompleteCellType: Int {

    case category
    case client
    case noProject
    case workspace
    case project
    case task
    case timeEntryFullData
    case unknown

    init(item: AutocompleteItem) {
        let type = item.type
        if type == -1 {
            self = .category
            return
        }
        if type == -2 {
            self = .client
            return
        }
        if type == 2 && item.projectID == 0 && item.projectGUID == nil {
            self = .noProject
            return
        }
        if type == -3 {
            self = .workspace
            return
        }
        if type == 2 {
            self = .project
            return
        }
        if type == 1 {
            self = .task
            return
        }
        if type == 0 {
            self = .timeEntryFullData
            return
        }
        self = .unknown
    }
}
