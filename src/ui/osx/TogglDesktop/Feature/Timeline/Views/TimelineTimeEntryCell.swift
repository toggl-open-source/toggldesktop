//
//  TimelineTimeEntryCell.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 6/21/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

protocol TimelineTimeEntryCellDelegate: class {

    func timeEntryCellShouldChangeFirstEntryStopTime(for entry: TimelineTimeEntry, sender: TimelineTimeEntryCell)
    func timeEntryCellShouldChangeLastEntryStartTime(for entry: TimelineTimeEntry, sender: TimelineTimeEntryCell)
}

final class CursorView: NSBox {

    var cursor: NSCursor? {
        didSet {
            resetCursorRects()
        }
    }

    override func resetCursorRects() {
        if let cursor = cursor {
            addCursorRect(bounds, cursor: cursor)
        } else {
            super.resetCursorRects()
        }
    }
}

final class TimelineTimeEntryCell: TimelineBaseCell {

    // MARK: Variables

    weak var delegate: TimelineTimeEntryCellDelegate?
    private(set) var timeEntry: TimelineTimeEntry!
    private lazy var timeEntryMenu = TimelineTimeEntryMenu()

    // MARK: OUTLET

    @IBOutlet weak var titleLbl: NSTextField!
    @IBOutlet weak var projectStackView: NSStackView!
    @IBOutlet weak var dotColorBox: DotImageView!
    @IBOutlet weak var projectLbl: ProjectTextField!
    @IBOutlet weak var bottomStackView: NSStackView!
    @IBOutlet weak var clientNameLbl: NSTextField!
    @IBOutlet weak var billableImageView: NSImageView!
    @IBOutlet weak var tagImageView: NSImageView!
    @IBOutlet weak var iconStackView: NSStackView!
    @IBOutlet weak var dateLbl: NSTextField!
    @IBOutlet weak var durationLbl: NSTextField!

    // MARK: View

    override func viewDidLoad() {
        super.viewDidLoad()
        initCommon()
        initTrackingArea()
    }

    // MARK: Public

    func config(for timeEntry: TimelineTimeEntry) {
        self.timeEntry = timeEntry
        renderColor(with: timeEntry.color, isSmallEntry: timeEntry.isSmall)
        populateInfo()
    }

    override func mouseEntered(with event: NSEvent) {
        super.mouseEntered(with: event)
        foregroundBox.resetCursorRects()
    }

    override func mouseExited(with event: NSEvent) {
        super.mouseExited(with: event)
        foregroundBox.resetCursorRects()
    }

    override func rightMouseDown(with event: NSEvent) {
        super.rightMouseDown(with: event)

        if let timeEntry = timeEntry, timeEntry.isOverlap {
            NSMenu.popUpContextMenu(timeEntryMenu, with: event, for: self.view)
        }
    }

    private func populateInfo() {
        guard let timeEntry = timeEntry else { return }
        backgroundBox.isHidden = !timeEntry.hasDetailInfo

        if timeEntry.hasDetailInfo {
            let item = timeEntry.timeEntry
            updateLabels(item)
        }
     }

    func hideLabelComponents() {
        guard let timeEntry = timeEntry,
            timeEntry.hasDetailInfo else { return }

        // Force update frame
        view.setNeedsDisplay(view.frame)
        view.displayIfNeeded()

        // Hide if it too small
        backgroundBox.isHidden = view.frame.height <= 10

        // Hide if some views is out of bounds
        let components: [NSView] = [titleLbl, projectStackView, bottomStackView, dateLbl, durationLbl]
        for view in components {
            let bottomFrame = CGRect(x: 0, y: 0, width: view.frame.width, height: 5)
            let isContain = view.frame.intersects(bottomFrame) || view.frame.origin.y < 0
            view.isHidden = isContain
        }
    }

    private func updateLabels(_ item: TimeEntryViewItem) {
        dateLbl.stringValue = "\(item.startTimeString ?? "") - \(item.endTimeString ?? "")"
        durationLbl.stringValue = item.duration
        tagImageView.isHidden = item.tags?.isEmpty ?? true
        billableImageView.isHidden = !item.billable
        if let description = item.descriptionName, !description.isEmpty {
            titleLbl.stringValue = description
            titleLbl.toolTip = description
        } else {
            titleLbl.stringValue = "No Description"
            titleLbl.toolTip = nil
        }

        // Projects
        if let project = item.projectLabel, !project.isEmpty {
            if let color = ConvertHexColor.hexCode(toNSColor: item.projectColor) {
                dotColorBox.isHidden = false
                dotColorBox.fill(with: color)
                projectLbl.textColor = color
            } else {
                dotColorBox.isHidden = true
                projectLbl.textColor = NSColor.labelColor
            }
            projectLbl.stringValue = project
            projectLbl.toolTip = project
        } else {
            dotColorBox.isHidden = true
            projectLbl.stringValue = "No Project"
            projectLbl.textColor = NSColor.labelColor
            projectLbl.toolTip = nil
        }

        // Client
        clientNameLbl.stringValue = item.clientLabel.isEmpty ? "No Client" : item.clientLabel
    }
}

// MARK: Private

extension TimelineTimeEntryCell {

    fileprivate func initCommon() {
        timeEntryMenu.menuDelegate = self
        if let cursorView = foregroundBox as? CursorView {
            cursorView.cursor = NSCursor.pointingHand
        }
    }
}

// MARK: TimelineTimeEntryMenuDelegate

extension TimelineTimeEntryCell: TimelineTimeEntryMenuDelegate {

    func shouldChangeFirstEntryStopTime() {
        guard let timeEntry = timeEntry else { return }
        delegate?.timeEntryCellShouldChangeFirstEntryStopTime(for: timeEntry, sender: self)
    }

    func shouldChangeLastEntryStartTime() {
        guard let timeEntry = timeEntry else { return }
        delegate?.timeEntryCellShouldChangeLastEntryStartTime(for: timeEntry, sender: self)
    }
}
