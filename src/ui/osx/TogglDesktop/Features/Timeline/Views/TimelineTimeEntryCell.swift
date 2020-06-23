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

/// Responsible for presenting the basic Timeline Time Entry
/// According to the design, there is three types of TimelineTimeEntryCell
/// 1 - Nomal Time Entry (single column, no Info bubble)
/// 2 - Expanded Time Entry (Has single column and Info Bubble)
/// 3 - Current Running Time Entry (It's the second one, but only have top rounded corners)
final class TimelineTimeEntryCell: TimelineBaseCell {

    // MARK: Variables

    weak var menuDelegate: TimelineTimeEntryCellDelegate?
    private(set) var timeEntry: TimelineTimeEntry!
    private lazy var timeEntryMenu = TimelineTimeEntryMenu()

    /// The view that the NSPopover should be presents
    var popoverView: NSView {
        if isSmallSize {
            return foregroundBox
        }
        guard let backgroundBox = backgroundBox else { return view }
        return backgroundBox.isHidden ? foregroundBox : view
    }

    /// Update the border for highlight items
    /// Hightlight items will have lighter color than the background
    var isHighlight: Bool = false {
        didSet {
            backgroundBox?.borderColor = (isHighlight ? foregroundBox.backgroundColor : backgroundColor) ?? foregroundBox.backgroundColor
        }
    }

    override var isResizable: Bool { return true }
    override var isHoverable: Bool { return true }

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
    @IBOutlet weak var innerBackgroundBox: CornerBoxView! // Prevent transparent background color
    
    // MARK: View

    override func viewDidLoad() {
        super.viewDidLoad()

        initCommon()
    }

    override func prepareForReuse() {
        super.prepareForReuse()
        isHighlight = false
    }

    // MARK: Public

    /// Main func to populate all information Time Entry
    /// - Parameter timeEntry: Current Time Entry
    func config(for timeEntry: TimelineTimeEntry) {
        self.timeEntry = timeEntry
        renderColor(with: timeEntry.color)
        populateInfo()
        handleRunningTimeEntry()
    }

    /// Populate the Time Entry data
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

    /// Handle logic to dismiss the Info Bubble
    private func hideOutOfBoundControls() {
        guard let timeEntry = timeEntry,
            timeEntry.hasDetailInfo else { return }

        // Hide if it too small
        hideBackgroundViews(isHidden: isSmallSize)

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

    /// Update all lables
    /// - Parameter item: Time Entry
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

    /// Update the corners for the running Time Entry
    private func handleRunningTimeEntry() {
        let isRunning = timeEntry?.timeEntry.isRunning() ?? false
        let corner: Corners = isRunning ? [.topLeft, .topRight] : [.bottomLeft, .bottomRight, .topLeft, .topRight]
        backgroundBox?.corners = corner
        foregroundBox.corners = corner
    }
}

// MARK: Private

extension TimelineTimeEntryCell {

    fileprivate func initCommon() {
        timeEntryMenu.menuDelegate = self
        view.menu = timeEntryMenu
        view.menu?.delegate = self
    }

    private func hideBackgroundViews(isHidden: Bool) {
        backgroundBox?.isHidden = isHidden
        innerBackgroundBox.isHidden = isHidden
    }
}

// MARK: TimelineTimeEntryMenuDelegate

extension TimelineTimeEntryCell: TimelineTimeEntryMenuDelegate {
    
    func timelineMenuContinue(_ timeEntry: TimelineTimeEntry) {
        menuDelegate?.timeEntryCellShouldContinue(for: timeEntry, sender: self)
    }

    func timelineMenuStartEntry(_ timeEntry: TimelineTimeEntry) {
        menuDelegate?.timeEntryCellShouldStartNew(for: timeEntry, sender: self)
    }

    func timelineMenuDelete(_ timeEntry: TimelineTimeEntry) {
        menuDelegate?.timeEntryCellShouldDelete(for: timeEntry, sender: self)
    }

    func timelineMenuChangeFirstEntryStopTime(_ timeEntry: TimelineTimeEntry) {
        menuDelegate?.timeEntryCellShouldChangeFirstEntryStopTime(for: timeEntry, sender: self)
    }

    func timelineMenuChangeLastEntryStartTime(_ timeEntry: TimelineTimeEntry) {
        menuDelegate?.timeEntryCellShouldChangeLastEntryStartTime(for: timeEntry, sender: self)
    }
}

// MARK: NSMenuDelegate

extension TimelineTimeEntryCell: NSMenuDelegate {

    func menuWillOpen(_ menu: NSMenu) {
        guard let timeEntry = timeEntry else { return }
        // disable some menu if it's overlapped item
        timeEntryMenu.timeEntry = timeEntry
        timeEntryMenu.isOverlapMenu = timeEntry.isOverlap
        isHighlight = true
    }

    func menuDidClose(_ menu: NSMenu) {
        isHighlight = false
    }
}
