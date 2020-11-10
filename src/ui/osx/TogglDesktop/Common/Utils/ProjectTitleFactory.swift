//
//  ProjectTitleFactory.swift
//  TogglDesktop
//
//  Created by Andrew Nester on 30.07.2020.
//  Copyright © 2020 Toggl. All rights reserved.
//

import Foundation

/// Builds `NSAttributedString` to display project information to the user.
@objc
class ProjectTitleFactory: NSObject {

    private let fontSize: CGFloat = 12
    private let leadingDotFontSize: CGFloat = 10
    private let defaultTextColor = Color.greyText.color

    @objc
    func title(for entry: TimeEntryViewItem) -> NSAttributedString {
        guard let project = entry.projectLabel else {
            return NSAttributedString()
        }

        return title(withProject: project,
                     task: entry.taskLabel,
                     client: entry.clientLabel,
                     projectColor: ConvertHexColor.hexCode(toNSColor: entry.projectColor))
    }

    @objc
    func title(withProject project: String,
               task: String? = nil,
               client: String? = nil,
               projectColor: NSColor,
               clientColor: NSColor? = nil) -> NSAttributedString {

        let dotAttributes = titleAttributes(withFontSize: leadingDotFontSize, foregroundColor: projectColor.getAdaptiveColorForShape())
        let string = NSMutableAttributedString(string: "●  ", attributes: dotAttributes)

        let baseAttributes = titleAttributes(withFontSize: fontSize, foregroundColor: projectColor.getAdaptiveColorForText())
        let projectName = NSMutableAttributedString(string: project, attributes: baseAttributes)
        string.append(projectName)

        if let task = task, !task.isEmpty {
            let taskName = NSAttributedString(string: ": \(task)", attributes: baseAttributes)
            string.append(taskName)
        }

        if let client = client, !client.isEmpty {
            let clientAttr = titleAttributes(withFontSize: fontSize, foregroundColor: clientColor)
            let clientName = NSAttributedString(string: " • \(client)", attributes: clientAttr)
            string.append(clientName)
        }

        return string
    }

    // MARK: - Private

    private func titleAttributes(withFontSize size: CGFloat, foregroundColor: NSColor? = nil) -> [NSAttributedString.Key: Any] {
        let paragraphStyle = NSMutableParagraphStyle()
        paragraphStyle.lineBreakMode = .byTruncatingTail
        paragraphStyle.alignment = .center

        var attributes: [NSAttributedString.Key: Any] = [
            .font: NSFont.systemFont(ofSize: size),
            .paragraphStyle: paragraphStyle
        ]

        if let color = foregroundColor {
            attributes[.foregroundColor] = color
        } else {
            attributes[.foregroundColor] = defaultTextColor
        }

        return attributes
    }
}
