//
//  WorkspaceCellView.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 4/1/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

final class WorkspaceCellView: HoverTableCellView {

    static let cellHeight: CGFloat = 34.0

    // MARK: OUTLET

    @IBOutlet weak var titleTextField: NSTextField!

    // MARK: Public

    func render(_ item: Workspace) {
        titleTextField.stringValue = item.name
    }
}
