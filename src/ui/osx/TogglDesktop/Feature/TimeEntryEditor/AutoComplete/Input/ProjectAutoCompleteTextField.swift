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
        view.delegate = self
        return view
    }()

    override func didTapOnCreateButton() {
        super.didTapOnCreateButton()

        // Update content
        updateWindowContent(with: projectCreationView, height: projectCreationView.suitableHeight)

        // Set text and focus
        projectCreationView.setTitleAndFocus(self.stringValue)
    }
}

// MARK: ProjectCreationViewDelegate

extension ProjectAutoCompleteTextField: ProjectCreationViewDelegate {

    func projectCreationDidAdd() {
        closeSuggestion()
    }

    func projectCreationDidCancel() {
        closeSuggestion()
    }

    func projectCreationDidUpdateSize() {
        updateWindowContent(with: projectCreationView, height: projectCreationView.suitableHeight)
    }
}
