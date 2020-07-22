//
//  ClientCellView.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 3/29/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

final class ClientCellView: HoverTableCellView {

    static let cellHeight: CGFloat = 34.0

    // MARK: OUTLET

    @IBOutlet weak var titleTextField: NSTextField!

    // MARK: Public

    func render(_ item: Client) {
        titleTextField.stringValue = item.name
    }
}
