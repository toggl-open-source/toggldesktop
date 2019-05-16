//
//  DescriptionDataSource.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 5/16/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

final class DescriptionTimeEntry {

    let name: String
    let item: AutocompleteItem

    init(item: AutocompleteItem) {
        self.item = item
        self.name = item.descriptionTitle
    }
}

final class DescriptionDataSource: AutoCompleteViewDataSource {

    private struct Constants {

        static let HeaderCell = NSUserInterfaceItemIdentifier("ProjectHeaderCellView")
        static let HeaderNibName = NSNib.Name("ProjectHeaderCellView")
        static let WorkspaceCell = NSUserInterfaceItemIdentifier("ProjectWorksapceCellView")
        static let WorkspaceNibName = NSNib.Name("ProjectWorksapceCellView")
        static let DescriptionContentCell = NSUserInterfaceItemIdentifier("DescriptionContentCellView")
        static let DescriptionNibName = NSNib.Name("DescriptionContentCellView")
    }

    // MARK: Variables

    override func setup(with textField: AutoCompleteTextField) {
        super.setup(with: textField)
        tableView.allowsEmptySelection = true
        autoCompleteView.setCreateButtonSectionHidden(true)
    }

    override func registerCustomeCells() {
        tableView.register(NSNib(nibNamed: Constants.HeaderNibName, bundle: nil),
                           forIdentifier: Constants.HeaderCell)
        tableView.register(NSNib(nibNamed: Constants.DescriptionNibName, bundle: nil),
                           forIdentifier: Constants.DescriptionContentCell)
        tableView.register(NSNib(nibNamed: Constants.WorkspaceNibName, bundle: nil),
                           forIdentifier: Constants.WorkspaceCell)
    }

    override func render(with items: [Any]) {
        super.render(with: items)
        autoCompleteView.isHidden = items.isEmpty
    }

    override func filter(with text: String) {

        // show all
        if text.isEmpty {
            render(with: DescriptionTimeEntryStorage.shared.items)
            return
        }

        // Filter
        let filterItems = DescriptionTimeEntryStorage.shared.filter(with: text)
        render(with: filterItems)
    }

    override func keyboardDidEnter() {

        // If there is no selection
        // Just get the stringValue
        if items[safe: tableView.selectedRow] == nil {

            // Close and update
            textField.closeSuggestion()
            textField.autoCompleteDelegate?.autoCompleteTextFieldDidEndEditing(textField)
        }
    }

    // MARK: Public

    override func tableView(_ tableView: NSTableView, viewFor tableColumn: NSTableColumn?, row: Int) -> NSView? {
        let item = items[row]
        switch item {
        case is ProjectHeaderItem:
            let view = tableView.makeView(withIdentifier: Constants.HeaderCell, owner: self) as! ProjectHeaderCellView
            view.renderAsTimeEntry()
            return view
        case let timeEntry as DescriptionTimeEntry:
            let view = tableView.makeView(withIdentifier: Constants.DescriptionContentCell, owner: self) as! DescriptionContentCellView
            view.render(timeEntry)
            return view
        case let worksapce as ProjectWorkspaceItem:
            let view = tableView.makeView(withIdentifier: Constants.WorkspaceCell, owner: self) as! ProjectWorksapceCellView
            view.render(worksapce)
            return view
        default:
            return nil
        }
    }

    override func tableView(_ tableView: NSTableView, heightOfRow row: Int) -> CGFloat {
        let item = items[row]
        switch item {
        case is ProjectHeaderItem:
            return ProjectHeaderCellView.cellHeight
        case is DescriptionTimeEntry:
            return DescriptionContentCellView.cellHeight
        case is ProjectWorkspaceItem:
            return ProjectWorksapceCellView.cellHeight
        default:
            return 0
        }
    }

    override func tableView(_ tableView: NSTableView, shouldSelectRow row: Int) -> Bool {
        let item = items[row]
        if item is DescriptionTimeEntry {
            return true
        }
        return false
    }
}
