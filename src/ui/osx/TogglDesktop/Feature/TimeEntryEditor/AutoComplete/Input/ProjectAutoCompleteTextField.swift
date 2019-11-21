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
    private(set) var lastProjectGUID: String?
    var projectItem: ProjectContentItem? {
        didSet {
            guard let project = projectItem else { return }
            var label = project.name
            if project.item.taskID != 0 && project.item.projectID != 0 {
                label = label + ": " + project.item.taskLabel
            }
            stringValue = label
            layoutProject(with: project.name)
            applyColor(with: project.colorHex)
        }
    }
    var dotImageView: DotImageView?
    private lazy var projectCreationView: ProjectCreationView = {
        let view = ProjectCreationView.xibView() as ProjectCreationView
        view.delegate = self
        return view
    }()

    override func awakeFromNib() {
        super.awakeFromNib()
        self.allowsEditingTextAttributes = true
    }

    func setTimeEntry(_ timeEntry: TimeEntryViewItem) {
        projectCreationView.selectedTimeEntry = timeEntry

        if currentEditor() == nil {
            var label = timeEntry.projectLabel
            if timeEntry.taskID != 0 && timeEntry.projectID != 0 {
                label = label! + ": " + timeEntry.taskLabel
            }
            stringValue = label ?? ""
            layoutProject(with: stringValue)
            applyColor(with: timeEntry.projectColor)
        }
    }

    override func didTapOnCreateButton() {
        super.didTapOnCreateButton()

        // Update content
        updateWindowContent(with: projectCreationView, height: projectCreationView.suitableHeight)

        // Set text and focus
        projectCreationView.setTitleAndFocus(self.stringValue)
    }

    private func layoutProject(with name: String) {
        guard let cell = self.cell as? VerticallyCenteredTextFieldCell else { return }
        if name.isEmpty {
            cell.leftPadding = 10
            cell.rightPadding = 0
            dotImageView?.isHidden = true
        } else {
            cell.rightPadding = 35.0
            cell.leftPadding = 28.0
            dotImageView?.isHidden = false

            // If we're editing this field
            // Resign and make first responder to render the currentEditor
            if self.currentEditor() == window?.firstResponder {
                window?.makeFirstResponder(nil)
                window?.makeFirstResponder(self)
            }
        }
        setNeedsDisplay()
        displayIfNeeded()
    }

    private func applyColor(with hex: String) {
        guard let color = ConvertHexColor.hexCode(toNSColor: hex) else { return }
        let visibleColor = color.visibleColor()
        dotImageView?.fill(with: visibleColor)
        let font = self.font ?? NSFont.systemFont(ofSize: 14.0)
        let parap = NSMutableParagraphStyle()
        parap.lineBreakMode = .byTruncatingTail
        let att: [NSAttributedString.Key: Any] = [NSAttributedString.Key.font: font,
                                                  NSAttributedString.Key.foregroundColor: visibleColor,
                                                  NSAttributedString.Key.paragraphStyle: parap]
        attributedStringValue = NSAttributedString(string: stringValue, attributes: att)
    }
}

// MARK: ProjectCreationViewDelegate

extension ProjectAutoCompleteTextField: ProjectCreationViewDelegate {

    func projectCreationDidAdd(with name: String, color: String, projectGUID: String) {
        lastProjectGUID = projectGUID
        stringValue = name
        layoutProject(with: name)
        applyColor(with: color)
        closeSuggestion()
    }

    func projectCreationDidCancel() {
        closeSuggestion()
    }

    func projectCreationDidUpdateSize() {
        updateWindowContent(with: projectCreationView, height: projectCreationView.suitableHeight)
    }
}
