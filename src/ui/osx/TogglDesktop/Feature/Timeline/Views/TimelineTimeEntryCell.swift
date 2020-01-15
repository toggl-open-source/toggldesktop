//
//  TimelineTimeEntryCell.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 6/21/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

protocol TimelineTimeEntryCellDelegate: class {

    func timeEntryCellShouldContinue(for entry: TimelineTimeEntry, sender: TimelineTimeEntryCell)
    func timeEntryCellShouldStartNew(for entry: TimelineTimeEntry, sender: TimelineTimeEntryCell)
    func timeEntryCellShouldDelete(for entry: TimelineTimeEntry, sender: TimelineTimeEntryCell)
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

    private var isSmallSize: Bool {
        return view.frame.height <= 10
    }

    var popoverView: NSView {
        if isSmallSize {
            return foregroundBox
        }
        guard let backgroundBox = backgroundBox else { return view }
        return backgroundBox.isHidden ? foregroundBox : view
    }

    var isHighlight: Bool = false {
        didSet {
            backgroundBox?.borderColor = (isHighlight ? foregroundBox.fillColor : backgroundColor) ?? foregroundBox.fillColor
        }
    }

    // MARK: OUTLET

    @IBOutlet weak var titleLbl: NSTextField!
    @IBOutlet weak var projectStackView: NSStackView!
    @IBOutlet weak var dotColorBox: DotImageView!
    @IBOutlet weak var projectLbl: ProjectTextField!
    @IBOutlet weak var clientNameLbl: NSTextField!
    @IBOutlet weak var billableImageView: NSImageView!
    @IBOutlet weak var tagImageView: NSImageView!
    @IBOutlet weak var iconStackView: NSStackView!
    @IBOutlet weak var durationLbl: NSTextField!
    @IBOutlet weak var mainStackView: NSStackView!

    // MARK: View

    override func viewDidLoad() {
        super.viewDidLoad()

        initCommon()
        initTrackingArea()
    }

    override func prepareForReuse() {
        super.prepareForReuse()
        isHighlight = false
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

    private func populateInfo() {
        guard let timeEntry = timeEntry else { return }
        backgroundBox?.isHidden = !timeEntry.hasDetailInfo
        isHighlight = false

        if timeEntry.hasDetailInfo {
            let item = timeEntry.timeEntry
            updateLabels(item)
            hideOutOfBoundControls()
        }
     }

    private func hideOutOfBoundControls() {
        guard let timeEntry = timeEntry,
            timeEntry.hasDetailInfo else { return }

        // Hide if it too small
        backgroundBox?.isHidden = isSmallSize

        // Set initial state
        let topPadding: CGFloat = 5
        let bottomBarHeight: CGFloat = 5
        let bubbleHeight = self.view.frame.height

        // Prepare controls need to check
        var components: [NSView] = [titleLbl]
        if !projectStackView.isHidden { components.append(projectStackView) }
        if !clientNameLbl.isHidden { components.append(clientNameLbl) }
        components.append(iconStackView)

        // Force drawing to get the latest frame
        view.setNeedsDisplay(view.frame)
        view.displayIfNeeded()

        // Hide if some views is out of bounds
        for view in components {
            let bottomFrame = CGRect(x: 0, y: mainStackView.frame.height - bubbleHeight + topPadding, width: self.view.frame.width, height: bottomBarHeight)
            let isContain = view.frame.intersects(bottomFrame) || view.frame.origin.y <= bottomFrame.origin.y
            view.isHidden = isContain
        }
    }

    private func updateLabels(_ item: TimeEntryViewItem) {
        iconStackView.isHidden = false
        durationLbl.stringValue = item.dateDuration
        tagImageView.isHidden = item.tags?.isEmpty ?? true
        billableImageView.isHidden = !item.billable

        // Desciption
        if let description = item.descriptionName, !description.isEmpty {
            titleLbl.stringValue = description
            titleLbl.toolTip = description
        } else {
            titleLbl.stringValue = "No Description"
            titleLbl.toolTip = nil
        }

        // Projects
        if let project = item.projectLabel, !project.isEmpty {
            projectStackView.isHidden = false
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
            projectStackView.isHidden = true
        }

        // Client
        if let client = item.clientLabel, !client.isEmpty {
            clientNameLbl.stringValue = client
            clientNameLbl.isHidden = false
        } else {
            clientNameLbl.isHidden = true
        }
    }
}

// MARK: Private

extension TimelineTimeEntryCell {

    fileprivate func initCommon() {
        timeEntryMenu.menuDelegate = self
        view.menu = timeEntryMenu
        view.menu?.delegate = self
        if let cursorView = foregroundBox as? CursorView {
            cursorView.cursor = NSCursor.pointingHand
        }
    }
}

// MARK: TimelineTimeEntryMenuDelegate

extension TimelineTimeEntryCell: TimelineTimeEntryMenuDelegate {
    
    func timelineMenuContinue() {
        guard let timeEntry = timeEntry else { return }
        delegate?.timeEntryCellShouldContinue(for: timeEntry, sender: self)
    }

    func timelineMenuStartEntry() {
        guard let timeEntry = timeEntry else { return }
        delegate?.timeEntryCellShouldStartNew(for: timeEntry, sender: self)
    }

    func timelineMenuDelete() {
        guard let timeEntry = timeEntry else { return }
        delegate?.timeEntryCellShouldDelete(for: timeEntry, sender: self)
    }

    func timelineMenuChangeFirstEntryStopTime() {
        guard let timeEntry = timeEntry else { return }
        delegate?.timeEntryCellShouldChangeFirstEntryStopTime(for: timeEntry, sender: self)
    }

    func timelineMenuChangeLastEntryStartTime() {
        guard let timeEntry = timeEntry else { return }
        delegate?.timeEntryCellShouldChangeLastEntryStartTime(for: timeEntry, sender: self)
    }
}

// MARK: NSMenuDelegate

extension TimelineTimeEntryCell: NSMenuDelegate {

    func menuWillOpen(_ menu: NSMenu) {
        guard let timeEntry = timeEntry else { return }
        // disable some menu if it's overlapped item
        timeEntryMenu.isOverlapMenu = timeEntry.isOverlap
        isHighlight = true
    }

    func menuDidClose(_ menu: NSMenu) {
        isHighlight = false
    }
}
