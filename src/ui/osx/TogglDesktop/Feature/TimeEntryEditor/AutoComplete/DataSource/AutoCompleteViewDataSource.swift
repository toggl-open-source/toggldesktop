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

    // MARK: Init

    init(items: [Any], updateNotificationName: Notification.Name) {
        super.init()
        render(with: items)
        NotificationCenter.default.addObserver(self,
                                               selector: #selector(self.receiveDataSourceNotifiation(_:)),
                                               name: updateNotificationName,
                                               object: nil)
    }

    deinit {
        NotificationCenter.default.removeObserver(self)
    }

    // MARK: Public

    func registerCustomeCells() {
        // Should override
    }

    func render(with items: [Any]) {
        self.items = items
        tableView?.reloadData()
    }

    @objc func receiveDataSourceNotifiation(_ noti: Notification) {
        guard let items = noti.object as? [Any] else {
            return
        }
        render(with: items)
    }
}

extension AutoCompleteViewDataSource: NSTableViewDataSource, NSTableViewDelegate {

    func numberOfRows(in tableView: NSTableView) -> Int {
        return items.count
    }

    func tableView(_ tableView: NSTableView, viewFor tableColumn: NSTableColumn?, row: Int) -> NSView? {
        return nil
    }

    func tableView(_ tableView: NSTableView, heightOfRow row: Int) -> CGFloat {
        return 44.0
    }
}
