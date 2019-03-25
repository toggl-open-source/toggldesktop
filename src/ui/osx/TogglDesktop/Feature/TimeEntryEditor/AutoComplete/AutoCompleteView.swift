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

    // MARK: Variables
    private var dataSource: AutoCompleteViewDataSource!

    // MARK: Public

    func prepare(with dataSource: AutoCompleteViewDataSource) {
        self.dataSource = dataSource
        self.dataSource.prepare(self)
    }

    func filter(with text: String) {
        dataSource.filter(with: text)
    }

    func update(height: CGFloat) {
        tableViewHeight.constant = height
    }
    
    @IBAction func newProjectBtnOnTap(_ sender: Any) {
    }
}
