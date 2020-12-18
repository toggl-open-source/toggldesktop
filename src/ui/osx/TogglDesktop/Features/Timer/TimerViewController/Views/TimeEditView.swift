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
            DesktopLibraryBridge.shared().trackDurationDropdown(DurationDropdownActionTypeDateChange)
        }
    }

    private var isStartFieldChanged = false

    private var isCalendarVisible: Bool {
        calendarView.superview != nil && !calendarView.isHidden
    }

    private var notificationObserverToken: Any?
    private var isEscTriggeredClose = false

    private enum Constants {
        static let dayNameAttribute: [NSAttributedString.Key: Any] = {
            return [NSAttributedString.Key.font: NSFont.systemFont(ofSize: 14),
                    NSAttributedString.Key.foregroundColor: Color.blackText.color]
        }()

        static let calendarTopPadding: CGFloat = 8
    }

    // MARK: - Views

    @IBOutlet private weak var contentView: NSView!
    @IBOutlet private weak var startTextField: NSTextField!
    @IBOutlet private weak var todayButton: CursorButton!
    @IBOutlet private weak var datePicker: KeyboardDatePicker!

    @IBOutlet private weak var prevDayButton: CursorButton! {
        didSet {
            prevDayButton.setCanBecomeKeyView(true)
        }
    }

    @IBOutlet private weak var nextDayButton: CursorButton! {
        didSet {
            nextDayButton.setCanBecomeKeyView(true)
        }
    }

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

    required init?(coder: NSCoder) {
        dateValue = Date()
        super.init(coder: coder)
    }

    override func awakeFromNib() {
        super.awakeFromNib()

        update(with: dateValue)

        // nextDayButton is the last control in key view loop
        nextDayButton.didPressKey = { [weak self] key, modifiers in
            if key == .tab && !modifiers.contains(.shift) {
                self?.hideWindow()
            }
        }

        datePicker.keyOnAction = { [weak self] key in
            guard let self = self, !self.nextDayButton.isEnabled else { return }
            let resignedFocus = self.window?.firstResponder != self.datePicker
            if key == .tab && resignedFocus {
                self.hideWindow()
            }
        }

        notificationObserverToken = NotificationCenter.default.addObserver(
            forName: NSWindow.didResignKeyNotification,
            object: window,
            queue: .main
        ) { [unowned self] notification in
            let isCurrentWindow = (notification.object as? NSWindow) == self.window
            if isCurrentWindow && !self.isEscTriggeredClose {
                self.onStartTextChange?(self.startTextField.stringValue)
            }
            self.isEscTriggeredClose = false
            self.isStartFieldChanged = false
        }
    }

    override func becomeFirstResponder() -> Bool {
        startTextField.becomeFirstResponder()
    }

    override func keyDown(with event: NSEvent) {
        if let key = Key(rawValue: Int(event.keyCode)), key == .escape {
            isEscTriggeredClose = true
            hideWindow()
            return
        }
        super.keyDown(with: event)
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
        let day: String = {
            let formatter = DateFormatter()
            formatter.dateFormat = "EEE"
            return formatter.string(from: date)
        }()

        todayButton.attributedTitle = NSAttributedString(
            string: day + ",",
            attributes: Constants.dayNameAttribute
        )
        calendarViewControler.prepareLayout(with: date)
        datePicker.dateValue = date
        datePicker.maxDate = Date()
        nextDayButton.isEnabled = Calendar.current.isDate(date, inSameDayAs: Date()) == false
    }

    private func hideWindow() {
        window?.orderOut(nil)
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
            DesktopLibraryBridge.shared().trackDurationDropdown(DurationDropdownActionTypeStartTimeChange)
            isStartFieldChanged = false
        }
    }

    func controlTextDidChange(_ obj: Notification) {
        isStartFieldChanged = true
    }

    func control(_ control: NSControl, textView: NSTextView, doCommandBy commandSelector: Selector) -> Bool {
        if commandSelector == #selector(insertNewline(_:))
            || commandSelector == #selector(insertBacktab(_:)) {
            hideWindow()
        }

        if commandSelector == #selector(cancelOperation(_:)) {
            isEscTriggeredClose = true
            hideWindow()
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
