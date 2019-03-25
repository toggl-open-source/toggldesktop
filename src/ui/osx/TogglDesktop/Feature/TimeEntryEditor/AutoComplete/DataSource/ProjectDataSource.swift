//
//  ProjectDataSource.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 3/25/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

struct HeaderItem {

    let name: String

}

struct ProjectItem {

    let name: String
    let colorHex: String
}

final class ProjectDataSource: AutoCompleteViewDataSource {

    private struct Constants {

        static let HeaderCell = NSUserInterfaceItemIdentifier("HeaderCell")
        static let ProjectCell = NSUserInterfaceItemIdentifier("ProjectCell")
    }

    // MARK: Variables

    func registerCustomeCells() {

    }

    // MARK: Public

    override func tableView(_ tableView: NSTableView, rowViewForRow row: Int) -> NSTableRowView? {
        return nil
//        let item = items[row]
//        switch item {
//        case let header as HeaderItem:
//            let view = tableView.makeView(withIdentifier: Constants.HeaderCell, owner: self) as! AutoCompleteProjectHeaderView
//            return view
//        case let project as ProjectItem:
//            let view = tableView.makeView(withIdentifier: Constants.HeaderCell, owner: self) as! AutoCompleteProjectItemView
//            return view
//        default:
//            return nil
//        }
    }

    override func tableView(_ tableView: NSTableView, heightOfRow row: Int) -> CGFloat {
        let item = items[row]
        switch item {
        case is HeaderItem:
            return 23.0
        case is ProjectItem:
            return 35.0
        default:
            return 0
        }
    }
}
