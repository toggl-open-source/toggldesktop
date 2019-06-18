//
//  DatePickerView.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 6/17/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

protocol DatePickerViewDelegate: class {

    func datePickerOnChanged(_ sender: DatePickerView, date: Date)
    func datePickerShouldClose(_ sender: DatePickerView)
    func isTimeEntryRunning(_ sender: DatePickerView) -> Bool
    func shouldOpenCalendar(_ sender: DatePickerView) -> Bool
}

final class DatePickerView: NSView {

    // MARK: OUTLET

    @IBOutlet weak var datePickerView: KeyboardDatePicker!
    @IBOutlet weak var dayNameButton: CursorButton!
    @IBOutlet weak var nextDateBtn: NSButton!
    @IBOutlet weak var previousDateBtn: NSButton!
    @IBOutlet weak var dateSelectionBox: NSBox!

    // MARK: View

    weak var delegate: DatePickerViewDelegate?
    var currentDate: Date = Date() {
        didSet {
            populateDate()
        }
    }
    private lazy var calendarPopover: NoVibrantPopoverView = {
        let popover = NoVibrantPopoverView()
        popover.behavior = .semitransient
        popover.contentViewController = calendarViewControler
        return popover
    }()
    lazy var calendarViewControler: CalendarViewController = {
        let controller = CalendarViewController(nibName: NSNib.Name("CalendarViewController"), bundle: nil)
        controller.delegate = self
        return controller
    }()
    private lazy var dayNameAttribute: [NSAttributedString.Key : Any] = {
        return [NSAttributedString.Key.font : NSFont.systemFont(ofSize: 14),
                NSAttributedString.Key.foregroundColor: NSColor.labelColor]
    }()
    var isEnabled: Bool = true {
        didSet {
            datePickerView.isEnabled = isEnabled
        }
    }

    // MARK: View

    override func awakeFromNib() {
        super.awakeFromNib()

        initCommon()
    }

    // MARK: Public

    func updateNextKeyView() {
        previousDateBtn.nextKeyView = dayNameButton
        dayNameButton.nextKeyView = datePickerView
        datePickerView.nextKeyView = nextDateBtn
    }

    @IBAction func datePickerChanged(_ sender: Any) {
        delegate?.datePickerOnChanged(self, date: datePickerView.dateValue)
    }

    @IBAction func dayButtonOnTap(_ sender: Any) {
        guard let delegate = delegate,
            !delegate.isTimeEntryRunning(self) else { return }
        guard delegate.shouldOpenCalendar(self) else { return }
        calendarPopover.present(from: dateSelectionBox.bounds, of: dateSelectionBox, preferredEdge: .maxY)
    }

    @IBAction func nextDateBtnOnTap(_ sender: Any) {
        guard let nextDate = currentDate.nextDate(),
            let delegate = delegate,
            !delegate.isTimeEntryRunning(self) else {
            return
        }
        delegate.datePickerOnChanged(self, date: nextDate)
    }

    @IBAction func previousDateBtnOnTap(_ sender: Any) {
        guard let previousDate = currentDate.previousDate(),
            let delegate = delegate,
            !delegate.isTimeEntryRunning(self) else {
            return
        }
        delegate.datePickerOnChanged(self, date: previousDate)
    }
}

extension DatePickerView {

    fileprivate func initCommon() {
        dayNameButton.cursor = .pointingHand
        datePickerView.escapeKeyOnAction = {[weak self] in
            guard let strongSelf = self else { return }
            strongSelf.delegate?.datePickerShouldClose(strongSelf)
        }
    }

    fileprivate func populateDate() {
        datePickerView.dateValue = currentDate
        let dayName = currentDate.dayOfWeekString() ?? "Unknown"
        dayNameButton.attributedTitle = NSAttributedString(string: "\(dayName),", attributes: dayNameAttribute)
        calendarViewControler.prepareLayout(with: currentDate)
    }
}

// MARK: CalendarViewControllerDelegate

extension DatePickerView: CalendarViewControllerDelegate {

    func calendarViewControllerDidSelect(date: Date) {
        delegate?.datePickerOnChanged(self, date: date)
    }
}

