//
//  TagDataSource.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 4/5/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

final class TagDataSource: AutoCompleteViewDataSource {

    private struct Constants {

        static let CellID = NSUserInterfaceItemIdentifier("TagCellView")
        static let CellNibName = NSNib.Name("TagCellView")
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
            render(with: TagStorage.shared.tags)
            return
        }

        // Filter
        let filterItems = TagStorage.shared.filter(with: text)
        render(with: filterItems)
    }

    // MARK: Public

    override func tableView(_ tableView: NSTableView, viewFor tableColumn: NSTableColumn?, row: Int) -> NSView? {
        let item = items[row] as! Tag
        let view = tableView.makeView(withIdentifier: Constants.CellID, owner: self) as! TagCellView
        view.render(item)
        return view
    }

    override func tableView(_ tableView: NSTableView, heightOfRow row: Int) -> CGFloat {
        return TagCellView.cellHeight
    }
}
