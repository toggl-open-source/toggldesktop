//
//  AutoCompleteView.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 3/25/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

final class AutoCompleteView: NSView {

    // MARK: OUTLET

    @IBOutlet weak var tableView: NSTableView!
    @IBOutlet weak var tableViewHeight: NSLayoutConstraint!
    @IBOutlet weak var createNewProjectBtn: NSButton!

    // MARK: Public

    func prepare(with datasource: AutoCompleteViewDataSource) {
        datasource.tableView = tableView
        tableView.delegate = datasource
        tableView.dataSource = datasource
    }

    @IBAction func newProjectBtnOnTap(_ sender: Any) {
    }
}
