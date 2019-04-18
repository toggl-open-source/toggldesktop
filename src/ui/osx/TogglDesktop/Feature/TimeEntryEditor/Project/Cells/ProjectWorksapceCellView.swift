//
//  ProjectWorksapceCellView.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 4/10/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

final class ProjectWorksapceCellView: NSTableCellView {

    static let cellHeight: CGFloat = 26.0

    // MARK: OUTLET

    @IBOutlet weak var titleTextField: NSTextField!

    // MARK: Public

    func render(_ item: ProjectWorkspaceItem) {
        let name = item.name.isEmpty ? "No workspace" : item.name
        titleTextField.stringValue = name.uppercased()
    }
}
