//
//  ProjectAutoCompleteTextField.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 3/28/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

final class ProjectAutoCompleteTextField: AutoCompleteTextField {

    // MARK: Variables

    private lazy var projectCreationView: ProjectCreationView = {
        let view = ProjectCreationView.xibView() as ProjectCreationView
        return view
    }()

    override func didTapOnCreateButton() {
        updateWindowContent(with: projectCreationView, height: projectCreationView.suitableHeight)
    }
}
