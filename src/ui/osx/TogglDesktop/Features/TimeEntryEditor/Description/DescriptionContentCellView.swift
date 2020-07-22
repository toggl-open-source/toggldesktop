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
    @IBOutlet weak var projectTextField: ProjectTextField!
    @IBOutlet weak var dotView: DotImageView!
    
    // MARK: Public

    func render(_ item: DescriptionTimeEntry) {
        descriptionTextField.stringValue = item.name
        projectTextField.setTitleWithAutoComplete(item.item)
        setupDotView(item.item)

    }
    private func setupDotView(_ item: AutocompleteItem) {
        dotView.isHidden = true

        if let projectLabel = item.projectLabel, !projectLabel.isEmpty {
            dotView.isHidden = false
        }
        if let clientLabel = item.clientLabel, !clientLabel.isEmpty {
            dotView.isHidden = false
        }

        if let projectColor = ConvertHexColor.hexCode(toNSColor: item.projectColor), dotView.isHidden == false {
            dotView.isHidden = false
            dotView.fill(with: projectColor)
        } else {
            dotView.isHidden = true
        }
    }
}
