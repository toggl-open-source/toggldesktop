//
//  AutoCompleteProjectHeaderView.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 3/25/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

final class ProjectHeaderCellView: NSTableCellView {

    static let cellHeight: CGFloat = 26.0

    // MARK: OUTLET

    @IBOutlet weak var titleTextField: NSTextField!

    // MARK: Public

    func render(_ item: ProjectHeaderItem) {
        let name = item.name.isEmpty ? "No client" : item.name
        titleTextField.stringValue = name
    }

    func renderAsTimeEntry() {
        titleTextField.stringValue = "TIME ENTRIES"
    }
}
