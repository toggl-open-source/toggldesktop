//
//  TimeEditView.swift
//  TogglDesktop
//
//  Created by Andrew Nester on 30.10.2020.
//  Copyright © 2020 Toggl. All rights reserved.
//

import Cocoa

class TimeEditView: NSView {

    var onStartTextChange: ((String) -> Void)?
    var onStartDateChange: ((Date) -> Void)?

    var startStringValue: String {
        get {
            startTextField.stringValue
        }
        set {
            startTextField.stringValue = newValue
            isStartFieldChanged = false
        }
    }

    var startDateValue: Date {
        get {
            dateValue
        }
        set {
            update(with: newValue)
        }
    }

    private var dateValue: Date = Date() {
        didSet {
            if dateValue > Date() {
                dateValue = Date()
            }
            update(with: dateValue)
            onStartDateChange?(dateValue)
        }
    }

    private var isStartFieldChanged = false

    private var isCalendarVisible: Bool {
        calendarView.superview != nil && !calendarView.isHidden
    }

    private enum Constants {
        static let dayNameAttribute: [NSAttributedString.Key: Any] = {
            return [NSAttributedString.Key.font: NSFont.systemFont(ofSize: 14),
                    NSAttributedString.Key.foregroundColor: NSColor.togglBlackText]
        }()

        static let calendarTopPadding: CGFloat = 8
    }

    // MARK: - Views

    @IBOutlet private weak var contentView: NSView!
    @IBOutlet private weak var startTextField: NSTextField!
    @IBOutlet private weak var todayButton: CursorButton!
    @IBOutlet private weak var datePicker: KeyboardDatePicker!
    @IBOutlet private weak var dateBox: NSBox!

    @IBOutlet private weak var dateBoxBottomConstraint: NSLayoutConstraint!
    private var bottomCalendarConstraint: NSLayoutConstraint!

    private var calendarView: NSView { calendarViewControler.view }

    private lazy var calendarViewControler: CalendarViewController = {
        let controller = CalendarViewController(nibName: NSNib.Name("CalendarViewController"), bundle: nil)
        controller.delegate = self
        return controller
    }()

    // MARK: - Overrides

    override func awakeFromNib() {
        super.awakeFromNib()
        dateValue = Date()
    }

    override func becomeFirstResponder() -> Bool {
        startTextField.becomeFirstResponder()
    }

    // MARK: - Actions

    @IBAction func previousDayButtonClicked(_ sender: Any) {
        dateValue = dateValue.previousDate() ?? dateValue
    }

    @IBAction func nextDayButtonClicked(_ sender: Any) {
        dateValue = dateValue.nextDate() ?? dateValue
    }

    @IBAction func datePickerChanged(_ sender: Any) {
        dateValue = datePicker.dateValue
    }

    @IBAction func todayButtonClicked(_ sender: Any) {
        isCalendarVisible ? hideCalendar() : showCalendar()
    }

    // MARK: - Private

    private func update(with date: Date) {
        todayButton.attributedTitle = NSAttributedString(
            string: date.dayOfWeekString() ?? "Unknown",
            attributes: Constants.dayNameAttribute
        )
        calendarViewControler.prepareLayout(with: date)
        datePicker.dateValue = date
        datePicker.maxDate = Date()
    }

    private func showCalendar() {
        if calendarView.superview == nil {
            addCalendarSubview()
        }

        dateBoxBottomConstraint.priority = .defaultHigh
        bottomCalendarConstraint.priority = .required
        calendarView.isHidden = false
        window?.setContentSize(frame.size)
    }

    private func hideCalendar() {
        bottomCalendarConstraint.priority = .defaultHigh
        dateBoxBottomConstraint.priority = .required
        calendarView.isHidden = true
        window?.setContentSize(frame.size)
    }

    private func addCalendarSubview() {
        calendarView.translatesAutoresizingMaskIntoConstraints = false
        contentView.addSubview(calendarView)

        bottomCalendarConstraint = calendarView.bottomAnchor.constraint(equalTo: contentView.bottomAnchor)

        NSLayoutConstraint.activate([
            calendarView.topAnchor.constraint(equalTo: dateBox.bottomAnchor, constant: Constants.calendarTopPadding),
            bottomCalendarConstraint,
            calendarView.leadingAnchor.constraint(equalTo: contentView.leadingAnchor),
            calendarView.trailingAnchor.constraint(equalTo: contentView.trailingAnchor)
        ])
    }
}

// MARK: - NSTextFieldDelegate

extension TimeEditView: NSTextFieldDelegate {
    func controlTextDidEndEditing(_ obj: Notification) {
        if obj.object as? NSTextField == startTextField, isStartFieldChanged {
            onStartTextChange?(startStringValue)
            isStartFieldChanged = false
        }
    }

    func controlTextDidChange(_ obj: Notification) {
        isStartFieldChanged = true
    }

    func control(_ control: NSControl, textView: NSTextView, doCommandBy commandSelector: Selector) -> Bool {
        if commandSelector == #selector(insertTab(_:))
            || commandSelector == #selector(insertBacktab(_:))
            || commandSelector == #selector(cancelOperation(_:))
            || commandSelector == #selector(insertNewline(_:)) {
                window?.orderOut(nil)
        }
        return false
    }
}

// MARK: - CalendarViewControllerDelegate

extension TimeEditView: CalendarViewControllerDelegate {

    func calendarViewControllerDidSelect(date: Date) {
        dateValue = date
    }

    func calendarViewControllerDoneBtnOnTap() {
        hideCalendar()
    }
}
