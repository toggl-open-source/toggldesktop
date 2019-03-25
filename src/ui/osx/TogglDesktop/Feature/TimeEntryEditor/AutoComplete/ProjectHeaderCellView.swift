//
//  AutoCompleteProjectHeaderView.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 3/25/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

final class ProjectHeaderCellView: NSTableCellView {

    // MARK: OUTLET

    @IBOutlet weak var titleTextField: NSTextField!

    // MARK: Public

    func render(_ item: ProjectHeaderItem) {
        titleTextField.stringValue = item.name
    }

}
