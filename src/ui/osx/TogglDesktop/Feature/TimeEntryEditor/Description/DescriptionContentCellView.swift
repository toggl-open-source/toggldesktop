//
//  DescriptionContentCellView.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 5/16/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

final class DescriptionContentCellView: HoverTableCellView {

    static let cellHeight: CGFloat = 34.0

    // MARK: OUTLET

    @IBOutlet weak var descriptionTextField: NSTextField!

    // MARK: Public

    func render(_ item: DescriptionTimeEntry) {

    }
}
