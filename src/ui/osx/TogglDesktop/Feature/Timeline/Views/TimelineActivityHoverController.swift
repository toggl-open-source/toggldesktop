//
//  TimelineActivityHoverController.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 8/20/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

final class TimelineActivityHoverController: NSViewController {

    // MARK: OUTLET

    @IBOutlet weak var timeLbl: NSTextField!
    @IBOutlet weak var eventStackView: NSStackView!

    // MARK: View

    override func viewDidLoad() {
        super.viewDidLoad()

    }

    // MARK: Public

    func render(_ activity: TimelineActivity) {
        timeLbl.stringValue = "\(activity.startTimeStr) - \(activity.endTimeStr)"
        renderEvents(activity.events)
    }

    private func renderEvents(_ events: [TimelineEvent]) {

        // Remove all
        eventStackView.subviews.forEach { eventStackView.removeView($0) }

        // Build labels
        var eventLabels: [NSView] = []
        for event in events {
            let headerLabel = buildLabel(from: event, isSubEvent: false)
            let subLabels = event.subEvents.map { buildLabel(from: $0, isSubEvent: true) }

            // Appending
            eventLabels.append(headerLabel)
            eventLabels.append(contentsOf: subLabels)

            // Add padding
            let paddingView = buildPaddingView()
            eventLabels.append(paddingView)
        }

        // Add to stack
        eventLabels.forEach {
            eventStackView.addArrangedSubview($0)
        }
    }

    private func buildLabel(from event: TimelineEvent, isSubEvent: Bool) -> NSTextField {
        let textField = NSTextField(frame: NSRect.zero)
        textField.isEditable = false
        textField.isSelectable = true
        textField.drawsBackground = false
        textField.isBordered = false
        textField.font = NSFont.systemFont(ofSize: 14.0)
        textField.textColor = isSubEvent ? NSColor.secondaryLabelColor : NSColor.labelColor
        textField.stringValue = isSubEvent ? "\(event.durationStr) \(event.title)" : "\(event.durationStr) \(event.fileName)"
        textField.toolTip = "\(textField.stringValue) \(event.fileName)"
        textField.lineBreakMode = .byTruncatingTail
        return textField
    }

    private func buildPaddingView() -> NSView {
        let paddingView = NSView(frame: NSRect.zero)
        paddingView.translatesAutoresizingMaskIntoConstraints = false
        paddingView.heightAnchor.constraint(equalToConstant: 5).isActive = true
        return paddingView
    }
}
