//
//  WorkspaceDataSource.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 4/1/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

final class Workspace {

    let ID: UInt64
    let WID: UInt64
    let name: String
    let guid: String?
    let isPremium: Bool

    init(viewItem: ViewItem) {
        self.ID = viewItem.id
        self.WID = viewItem.wid
        self.name = viewItem.name ?? ""
        self.guid = viewItem.guid ?? nil
        self.isPremium = viewItem.premium
    }
}

final class WorkspaceDataSource: AutoCompleteViewDataSource {

    private struct Constants {

        static let CellID = NSUserInterfaceItemIdentifier("WorkspaceCellView")
        static let CellNibName = NSNib.Name("WorkspaceCellView")
    }

    // MARK: Variables

    override func setup(with textField: AutoCompleteTextField) {
        super.setup(with: textField)
        autoCompleteView.setCreateButtonSectionHidden(true)
    }

    override func registerCustomeCells() {
        tableView.register(NSNib(nibNamed: Constants.CellNibName, bundle: nil),
                           forIdentifier: Constants.CellID)
    }

    override func filter(with text: String) {

        // show all
        if text.isEmpty {
            render(with: WorkspaceStorage.shared.workspaces)
            return
        }

        // Filter
        let filterItems = WorkspaceStorage.shared.filter(with: text)
        render(with: filterItems)
    }

    // MARK: Public

    override func tableView(_ tableView: NSTableView, viewFor tableColumn: NSTableColumn?, row: Int) -> NSView? {
        let item = items[row] as! Workspace
        let view = tableView.makeView(withIdentifier: Constants.CellID, owner: self) as! WorkspaceCellView
        view.render(item)
        return view
    }

    override func tableView(_ tableView: NSTableView, heightOfRow row: Int) -> CGFloat {
        return WorkspaceCellView.cellHeight
    }
}
