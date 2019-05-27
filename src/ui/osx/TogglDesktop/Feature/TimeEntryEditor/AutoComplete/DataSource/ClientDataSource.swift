//
//  ClientDataSource.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 3/29/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

final class Client {

    static let noMatching = Client()
    let ID: UInt64
    let WID: UInt64
    let name:String
    let guid: String?
    let isEmpty: Bool
    let workspaceName: String

    init(viewItem: ViewItem) {
        self.ID = viewItem.id
        self.WID = viewItem.wid
        self.name = viewItem.name ?? ""
        self.guid = viewItem.guid ?? nil
        self.isEmpty = false
        self.workspaceName = viewItem.workspaceName ?? ""
    }

    init() {
        self.ID = 0
        self.WID = 0
        self.name = ""
        self.guid = nil
        self.isEmpty = true
        self.workspaceName = ""
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

    var selectedWorkspace: Workspace?
    
    override func registerCustomeCells() {
        tableView.register(NSNib(nibNamed: Constants.ClientCellIDNibName, bundle: nil),
                           forIdentifier: Constants.ClientCellID)
        tableView.register(NSNib(nibNamed: Constants.ClientEmptyIDNibName, bundle: nil),
                           forIdentifier: Constants.ClientEmptyCellID)
    }

    override func filter(with text: String) {

        // show all
        if text.isEmpty {
            let clients = ClientStorage.shared.getClients(at: selectedWorkspace)
            if clients.isEmpty {
                render(with: [Client.noMatching])
            } else {
                render(with: clients)
            }
            return
        }

        // Filter
        let filterItems = ClientStorage.shared.filter(with: text, at: selectedWorkspace)
        render(with: filterItems)
    }

    override func render(with items: [Any]) {
        super.render(with: items)

        // Hide create if it's has content
        if let first = items.first as? Client, first.isEmpty {
            autoCompleteView.setCreateButtonSectionHidden(false)
            autoCompleteView.updateTitleForCreateButton(with: "Create Client \"\(textField.stringValue)\"")
        } else {
            autoCompleteView.setCreateButtonSectionHidden(true)
        }

        // Enable
        autoCompleteView.createNewItemBtn.isEnabled = !textField.stringValue.isEmpty
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
