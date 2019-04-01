//
//  ClientDataSource.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 3/29/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

struct Client {

    static let noMatching = Client()
    let ID: UInt64
    let WID: UInt64
    let name:String
    let guid: String?
    let isEmpty: Bool

    init(viewItem: ViewItem) {
        self.ID = viewItem.id
        self.WID = viewItem.wid
        self.name = viewItem.name ?? ""
        self.guid = viewItem.guid ?? nil
        self.isEmpty = false
    }

    init() {
        self.ID = 0
        self.WID = 0
        self.name = ""
        self.guid = nil
        self.isEmpty = true
    }
}

final class ClientDataSource: AutoCompleteViewDataSource {

    private struct Constants {

        static let ClientCellID = NSUserInterfaceItemIdentifier("ClientCellView")
        static let ClientCellIDNibName = NSNib.Name("ClientCellView")

        static let ClientEmptyCellID = NSUserInterfaceItemIdentifier("NoClientCellView")
        static let ClientEmptyIDNibName = NSNib.Name("NoClientCellView")
    }

    // MARK: Variables

    override func registerCustomeCells() {
        tableView.register(NSNib(nibNamed: Constants.ClientCellIDNibName, bundle: nil),
                           forIdentifier: Constants.ClientCellID)
        tableView.register(NSNib(nibNamed: Constants.ClientEmptyIDNibName, bundle: nil),
                           forIdentifier: Constants.ClientEmptyCellID)
    }

    override func filter(with text: String) {

        // show all
        if text.isEmpty {
            render(with: ClientStorage.shared.clients)
            return
        }

        // Filter
        let filterItems = ClientStorage.shared.filter(with: text)
        render(with: filterItems)
    }

    override func render(with items: [Any]) {
        super.render(with: items)

        // Hide create if it's has content
        if let first = items.first as? Client, first.isEmpty {
            autoCompleteView.setCreateButtonSectionHidden(false)
            autoCompleteView.updateTitleForCreateButton(with: "Create Client ")
        } else {
            autoCompleteView.setCreateButtonSectionHidden(true)
        }
    }

    // MARK: Public

    override func tableView(_ tableView: NSTableView, viewFor tableColumn: NSTableColumn?, row: Int) -> NSView? {

        // Client cell
        let item = items[row]
        switch item {
        case let client as Client:
            if client.isEmpty {
                return tableView.makeView(withIdentifier: Constants.ClientEmptyCellID, owner: self) as! NoClientCellView
            }
            let view = tableView.makeView(withIdentifier: Constants.ClientCellID, owner: self) as! ClientCellView
            view.render(client)
            return view
        default:
            return nil
        }
    }

    override func tableView(_ tableView: NSTableView, heightOfRow row: Int) -> CGFloat {
        let item = items[row] as! Client
        if item.isEmpty {
            return NoClientCellView.cellHeight
        }
        return ClientCellView.cellHeight
    }

    override func tableView(_ tableView: NSTableView, shouldSelectRow row: Int) -> Bool {
        let item = items[row] as! Client
        if item.isEmpty {
            return false
        }
        return true
    }
}
