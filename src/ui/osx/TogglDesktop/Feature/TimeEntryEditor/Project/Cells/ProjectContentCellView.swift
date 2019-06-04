//
//  ProjectContentCellView.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 3/25/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

final class ProjectContentCellView: HoverTableCellView {

    static let cellHeight: CGFloat = 34.0

    // MARK: OUTLET

    @IBOutlet weak var dotImageView: DotImageView!
    @IBOutlet weak var projectTextField: ProjectTextField!

    // MARK: Public
    
    func render(_ item: ProjectContentItem) {
        projectTextField.renderClient = false
        projectTextField.renderTask = (item.item.type == 1)
        dotImageView.isHidden = projectTextField.renderTask
        projectTextField.setTitleWithAutoComplete(item.item)
        dotImageView.fill(with: ConvertHexColor.hexCode(toNSColor: item.colorHex))
    }
}
