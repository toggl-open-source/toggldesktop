//
//  DateSelectionView.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 6/17/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

protocol DateSelectionViewDelegate: class {

    func dateSelectionChanged(_ sender: DateSelectionView, date: Date)
}

final class DateSelectionView: NSView {

    // MARK: OUTLET

    @IBOutlet weak var datePickerView: KeyboardDatePicker!
    @IBOutlet weak var dayNameButton: CursorButton!
    @IBOutlet weak var nextDateBtn: NSButton!
    @IBOutlet weak var previousDateBtn: NSButton!
    @IBOutlet weak var dateSelectionBox: NSBox!

    // MARK: View

    weak var delegate: DateSelectionViewDelegate?
    var timeEntry: TimeEntryViewItem!
    private lazy var calendarPopover: NoVibrantPopoverView = {
        let popover = NoVibrantPopoverView()
        popover.behavior = .semitransient
        popover.contentViewController = calendarViewControler
        return popover
    }()
    private lazy var calendarViewControler: CalendarViewController = {
        let controller = CalendarViewController(nibName: NSNib.Name("CalendarViewController"), bundle: nil)
        controller.delegate = self
        return controller
    }()

    // MARK: View

    override func awakeFromNib() {
        super.awakeFromNib()
    }

    // MARK: Public

    @IBAction func datePickerChanged(_ sender: Any) {
        delegate?.dateSelectionChanged(self, date: datePickerView.dateValue)
    }

    @IBAction func dayButtonOnTap(_ sender: Any) {
        guard !timeEntry.isRunning() else { return }
        calendarPopover.present(from: dateSelectionBox.bounds, of: dateSelectionBox, preferredEdge: .maxY)
    }
}

// MARK: CalendarViewControllerDelegate

extension DateSelectionView: CalendarViewControllerDelegate {

    func calendarViewControllerDidSelect(date: Date) {
        delegate?.dateSelectionChanged(self, date: date)
    }
}

