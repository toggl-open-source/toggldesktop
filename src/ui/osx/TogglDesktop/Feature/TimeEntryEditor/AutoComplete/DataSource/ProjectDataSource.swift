//
//  ProjectDataSource.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 3/25/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

struct ProjectHeaderItem {

    let name: String
    let item: AutocompleteItem

    init(item: AutocompleteItem) {
        self.item = item
        self.name = item.clientLabel
    }
}

struct ProjectRowItem {

    let name: String
    let colorHex: String
    let item: AutocompleteItem

    init(item: AutocompleteItem) {
        self.item = item
        self.name = item.projectLabel
        self.colorHex = item.projectColor
    }
}

final class ProjectDataSource: AutoCompleteViewDataSource {

    private struct Constants {

        static let HeaderCell = NSUserInterfaceItemIdentifier("AutoCompleteProjectHeaderView")
        static let HeaderNibName = NSNib.Name("AutoCompleteProjectHeaderView")
        static let ProjectCell = NSUserInterfaceItemIdentifier("ProjectCell")
        static let ProjectNibName = NSNib.Name("AutoCompleteProjectItemView")
    }

    // MARK: Variables

    override func registerCustomeCells() {
        tableView?.register(NSNib(nibNamed: Constants.HeaderNibName, bundle: nil),
                            forIdentifier: Constants.HeaderCell)
        tableView?.register(NSNib(nibNamed: Constants.ProjectNibName, bundle: nil),
                            forIdentifier: Constants.ProjectCell)
    }

    // MARK: Public

    override func tableView(_ tableView: NSTableView, viewFor tableColumn: NSTableColumn?, row: Int) -> NSView? {
        let item = items[row]
        switch item {
        case let header as ProjectHeaderItem:
            let view = tableView.makeView(withIdentifier: Constants.HeaderCell, owner: self) as! AutoCompleteProjectHeaderView
            return view
        case let project as ProjectRowItem:
            let view = tableView.makeView(withIdentifier: Constants.HeaderCell, owner: self) as! AutoCompleteProjectItemView
            return view
        default:
            return nil
        }
    }

    override func tableView(_ tableView: NSTableView, heightOfRow row: Int) -> CGFloat {
        let item = items[row]
        switch item {
        case is ProjectHeaderItem:
            return 23.0
        case is ProjectRowItem:
            return 35.0
        default:
            return 0
        }
    }
}
