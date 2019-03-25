//
//  AutoCompleteViewDataSource.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 3/25/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

class AutoCompleteViewDataSource: NSObject {

    // MARK: Variables
    
    weak var tableView: NSTableView?
    private(set) var items: [Any] = []

    // MARK: Public

    func render(with items: [Any]) {
        self.items = items
        tableView?.reloadData()
    }
}

extension AutoCompleteViewDataSource: NSTableViewDataSource, NSTableViewDelegate {

    func numberOfRows(in tableView: NSTableView) -> Int {
        return items.count
    }

    func tableView(_ tableView: NSTableView, rowViewForRow row: Int) -> NSTableRowView? {
        return nil
    }


    func tableView(_ tableView: NSTableView, heightOfRow row: Int) -> CGFloat {
        return 44.0
    }
}
