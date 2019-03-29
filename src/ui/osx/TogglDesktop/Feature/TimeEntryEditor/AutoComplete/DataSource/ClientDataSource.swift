//
//  ClientDataSource.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 3/29/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

struct Client {

    let ID: UInt64
    let WID: UInt64
    let name:String
    let guid: String?

    init(viewItem: ViewItem) {
        self.ID = viewItem.id
        self.WID = viewItem.wid
        self.name = viewItem.name ?? ""
        self.guid = viewItem.guid ?? nil
    }
}

final class ClientDataSource: AutoCompleteViewDataSource {

    private struct Constants {

        static let ClientCellID = NSUserInterfaceItemIdentifier("ClientCellView")
        static let ClientCellIDNibName = NSNib.Name("ClientCellView")
    }

    // MARK: Variables

    override func registerCustomeCells() {
        tableView.register(NSNib(nibNamed: Constants.ClientCellIDNibName, bundle: nil),
                           forIdentifier: Constants.ClientCellID)
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

    // MARK: Public

    override func tableView(_ tableView: NSTableView, viewFor tableColumn: NSTableColumn?, row: Int) -> NSView? {
        let item = items[row]
        switch item {
        case let client as Client:
            let view = tableView.makeView(withIdentifier: Constants.ClientCellID, owner: self) as! ClientCellView
            view.render(client)
            return view
        default:
            return nil
        }
    }

    override func tableView(_ tableView: NSTableView, heightOfRow row: Int) -> CGFloat {
        return ClientCellView.cellHeight
    }
}
