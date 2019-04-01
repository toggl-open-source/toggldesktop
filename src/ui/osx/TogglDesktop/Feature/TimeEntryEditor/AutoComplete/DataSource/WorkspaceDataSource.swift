//
//  WorkspaceDataSource.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 4/1/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

struct Workspace {

    let ID: UInt64
    let WID: UInt64
    let name:String
    let guid: String?

    init(viewItem: ViewItem) {
        self.ID = viewItem.id
        self.WID = viewItem.wid
        self.name = viewItem.name ?? ""
        self.guid = viewItem.guid ?? nil
    }
}

final class WorkspaceDataSource: AutoCompleteViewDataSource {

}
