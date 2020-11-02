//
//  TimerDurationControl.swift
//  TogglDesktop
//
//  Created by Andrew Nester on 29.10.2020.
//  Copyright © 2020 Toggl. All rights reserved.
//

import Foundation

class TimerDurationControl: NSView {

    var onDurationTextChange: ((String) -> Void)?
    var onStartTextChange: ((String) -> Void)? {
        didSet {
            timeEditView.onStartTextChange = onStartTextChange
        }
    }

    /// Feature flag for duration dropdown. Set to `true` to test the current implementation.
    var isDurationDropdownEnabled = false

    var durationStringValue: String {
        get { durationTextField.stringValue }
        set { durationTextField.stringValue = newValue }
    }

    var startTimeStringValue: String {
        get { timeEditView.startStringValue }
        set { timeEditView.startStringValue = newValue }
    }

    var isEditing: Bool {
        durationTextField.currentEditor() != nil && window?.isKeyWindow == true
    }

    override var canBecomeKeyView: Bool { true }
    override var acceptsFirstResponder: Bool { true }
    override var needsPanelToBecomeKey: Bool { true }

    enum Action: Equatable {
        case enterPress
    }

    /// Called when control requests an action to be performed.
    /// Closure must return `true` if action was handled.
    var onPerformAction: (Action) -> Bool = { _ in return false }

    private enum Constants {
        static let initialDropdownSize = NSSize(width: 256, height: 148)
        static let dropdownOffset = NSPoint(x: 0, y: 4)
        static let minWindowPadding: CGFloat = 8
    }

    private var notificationObserverToken: Any?

    // MARK: UI Components

    @IBOutlet private weak var durationTextField: ResponderTextField!
    @IBOutlet private weak var backgroundBox: TimerContainerBox!

    private lazy var dropdownWindow: DropdownWindow = {
        return DropdownWindow(view: timeEditView)
    }()

    private let timeEditView = TimeEditView.xibView()

    // MARK: - Overrides

    override func awakeFromNib() {
        super.awakeFromNib()
        setup()
    }

    deinit {
        if let token = notificationObserverToken {
            NotificationCenter.default.removeObserver(token)
        }
    }

    override func becomeFirstResponder() -> Bool {
        durationTextField.nextKeyView = nextKeyView
        return durationTextField.becomeFirstResponder()
    }

    private func setup() {
        backgroundBox.setup(for: durationTextField)
        durationTextField.delegate = self

        durationTextField.observeBecomeFirstResponder(self) { [weak self] in
            self?.presentDropdown()
        }

        durationTextField.observeResignFirstResponder(self) { [weak self] in
            self?.closeDropdown()
        }

        timeEditView.onStartTextChange = onStartTextChange

        notificationObserverToken = NotificationCenter.default.addObserver(
            forName: NSWindow.didResignKeyNotification,
            object: window,
            queue: .main
        ) { [unowned self] notification in
            if (notification.object as? NSWindow) == self.window {
                self.onDurationTextChange?(self.durationTextField.stringValue)
            }
        }
    }

    override func viewWillStartLiveResize() {
        closeDropdown()
    }

    // MARK: - Private

    private func presentDropdown() {
        guard isDurationDropdownEnabled else {
            return
        }

        let windowRect = dropdownWindowRect(fromView: durationTextField, offset: Constants.dropdownOffset)
        dropdownWindow.setFrame(windowRect, display: false)
        dropdownWindow.setFrameTopLeftPoint(windowRect.origin)

        if dropdownWindow != window {
            window?.addChildWindow(dropdownWindow, ordered: .above)
        }
    }

    private func closeDropdown() {
        dropdownWindow.orderOut(nil)
    }

    private func dropdownWindowRect(fromView: NSView, offset: NSPoint = .zero) -> NSRect {
        var rect = NSRect(origin: .zero, size: Constants.initialDropdownSize)

        let fromPoint = NSPoint(x: fromView.bounds.minX + offset.x,
                                y: fromView.bounds.maxY + offset.y)
        var windowPoint = fromView.convert(fromPoint, to: nil)
        if let window = window, windowPoint.x + rect.width > window.frame.width {
            windowPoint.x = window.frame.width - rect.width - Constants.minWindowPadding
        }
        var screenPoint = CGPoint.zero
        if #available(OSX 10.12, *) {
            screenPoint = window?.convertPoint(toScreen: windowPoint) ?? .zero
        } else {
            screenPoint = window?.convertToScreen(NSRect(origin: windowPoint, size: .zero)).origin ?? .zero
        }
        rect.origin = screenPoint

        return rect
    }
}

extension TimerDurationControl: NSTextFieldDelegate {

    func controlTextDidEndEditing(_ obj: Notification) {
        onDurationTextChange?(durationTextField.stringValue)
    }

    func control(_ control: NSControl, textView: NSTextView, doCommandBy commandSelector: Selector) -> Bool {
        if commandSelector == #selector(insertNewline(_:)) {
            return onPerformAction(.enterPress)
        }

        if commandSelector == #selector(insertBacktab(_:)) {
            window?.selectKeyView(preceding: self)
            return true
        }

        if commandSelector == #selector(insertTab(_:)) {
            if dropdownWindow.isVisible {
                dropdownWindow.makeKeyAndOrderFront(nil)
                dropdownWindow.makeFirstResponder(dropdownWindow.contentView)
                return true
            }
        }

        if commandSelector == #selector(cancelOperation(_:)) {
            if dropdownWindow.isVisible {
                closeDropdown()
                return true
            }
        }

        if commandSelector == #selector(moveDown(_:)) {
            if !dropdownWindow.isVisible {
                presentDropdown()
                return true
            }
        }

        return false
    }
}
