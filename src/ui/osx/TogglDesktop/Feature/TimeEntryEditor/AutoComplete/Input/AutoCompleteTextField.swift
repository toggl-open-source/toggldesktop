//
//  AutoCompleteTextField.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 3/25/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

protocol AutoCompleteTextFieldDelegate: class {

    func autoCompleteDidTapOnCreateButton(_ sender: AutoCompleteTextField)
    func shouldClearCurrentSelection(_ sender: AutoCompleteTextField)
    func autoCompleteViewDidClose(_ sender: AutoCompleteTextField)
    func autoCompleteTextFieldDidEndEditing(_ sender: AutoCompleteTextField)
    func autoCompleteShouldCloseEditor(_ sender: AutoCompleteTextField)
}

class AutoCompleteTextField: UndoTextField, NSTextFieldDelegate, AutoCompleteViewDelegate {

    enum State {
        case expand
        case collapse
    }

    // MARK: OUTLET

    private lazy var arrowBtn: CursorButton = {
        let button = CursorButton(frame: NSRect.zero)
        button.image = NSImage(named: NSImage.Name("arrow-section-close"))
        button.cursor = .pointingHand
        button.target = self
        button.action = #selector(self.arrowBtnOnTap)
        button.setButtonType(NSButton.ButtonType.momentaryPushIn)
        button.isBordered = false
        button.bezelStyle = NSButton.BezelStyle.texturedRounded
        return button
    }()

    // MARK: Variables

    weak var autoCompleteDelegate: AutoCompleteTextFieldDelegate?
    lazy var autoCompleteWindow: AutoCompleteViewWindow = {
        let window = AutoCompleteViewWindow(view: autoCompleteView)
        window.isSeparateWindow = isSeperateWindow
        return window
    }()
    lazy var autoCompleteView: AutoCompleteView = AutoCompleteView.xibView()
    var isSeperateWindow: Bool { return true }
    private var _state = State.collapse {
        didSet {
            switch _state {
            case .collapse:
                arrowBtn.image = NSImage(named: NSImage.Name("arrow-section-open"))
                closeAutoComplete()
                autoCompleteDelegate?.autoCompleteViewDidClose(self)
            case .expand:
                arrowBtn.image = NSImage(named: NSImage.Name("arrow-section-close"))
                presentAutoComplete()
            }
        }
    }
    private(set) var state: State {
        get { return _state }
        set {
            guard newValue != state else { return }
            _state = newValue
        }
    }

    override var stringValue: String {
        didSet {
            toolTip = stringValue
        }
    }
    override var attributedStringValue: NSAttributedString {
        didSet {
            toolTip = attributedStringValue.string
        }
    }

    // MARK: Init

    override init(frame frameRect: NSRect) {
        super.init(frame: frameRect)
        initCommon()
    }

    required init?(coder: NSCoder) {
        super.init(coder: coder)
        initCommon()
    }

    // MARK: Public

    func controlTextDidEndEditing(_ obj: Notification) {
        state = .collapse

        // Remove currrent selection
        if stringValue.isEmpty {
            autoCompleteDelegate?.shouldClearCurrentSelection(self)
        }

        // Notify delegates
        autoCompleteDelegate?.autoCompleteTextFieldDidEndEditing(self)
    }

    func controlTextDidChange(_ obj: Notification) {
        handleTextDidChange()
    }

    @objc func arrowBtnOnTap() {
        switch state {
        case .collapse:
            state = .expand
            autoCompleteView.filter(with: "")
            if window?.firstResponder != self.currentEditor() {
                window?.makeFirstResponder(self)
            }
        case .expand:
            state = .collapse
        }
    }

    func openSuggestion() {
        state = .expand
    }
    
    func closeSuggestion() {
        state = .collapse
    }

    func resetText() {
        stringValue = ""
        handleTextDidChange()
    }

    func updateWindowContent(with view: NSView, height: CGFloat) {
        autoCompleteWindow.contentView = view
        let rect = windowFrameRect()
        autoCompleteWindow.layoutFrame(with: self, origin: rect.origin, size: rect.size)
        autoCompleteWindow.makeKey()
    }

    func handleTextDidChange() {
        state = .expand
        autoCompleteView.filter(with: self.stringValue)
    }

    private func closeAutoComplete() {
        autoCompleteWindow.cancel()
    }

    private func presentAutoComplete() {

        // Set auto complete table
        autoCompleteWindow.contentView = autoCompleteView
        autoCompleteWindow.setContentSize(autoCompleteView.frame.size)

        // Present if need
        if !autoCompleteWindow.isVisible {

            // Layout frame and position
            let rect = windowFrameRect()
            autoCompleteWindow.layoutFrame(with: self, origin: rect.origin, size: rect.size)

            // Add or make key
            if self.window != autoCompleteWindow {
                window?.addChildWindow(autoCompleteWindow,
                                       ordered: .above)
            } else {
                autoCompleteWindow.makeKeyAndOrderFront(nil)
            }
        }

        didPresentAutoComplete()
    }

    func windowFrameRect() -> CGRect {
        return frame
    }

    func didPresentAutoComplete() {
        // Override
    }

    func didTapOnCreateButton() {
        autoCompleteDelegate?.autoCompleteDidTapOnCreateButton(self)
    }
}

// MARK: Private

extension AutoCompleteTextField {

    fileprivate func initCommon() {
        delegate = self
        autoCompleteView.delegate = self
        _state = .collapse
    }

    func layoutArrowBtn(with parentView: NSView) {
        arrowBtn.translatesAutoresizingMaskIntoConstraints = false
        parentView.addSubview(arrowBtn)
        arrowBtn.centerYAnchor.constraint(equalTo: centerYAnchor, constant: 0).isActive = true
        arrowBtn.trailingAnchor.constraint(equalTo: trailingAnchor, constant: -10).isActive = true
        arrowBtn.widthAnchor.constraint(equalToConstant: 20).isActive = true
        arrowBtn.heightAnchor.constraint(equalToConstant: 20).isActive = true
    }

    func control(_ control: NSControl, textView: NSTextView, doCommandBy commandSelector: Selector) -> Bool {
        guard let currentEvent = NSApp.currentEvent else {
            return false
        }

        // Enter
        if commandSelector == #selector(NSResponder.insertNewline(_:)) {
            if state == .expand {
                return autoCompleteView.tableView.handleKeyboardEvent(currentEvent)
            }
        }

        // Escape
        if commandSelector == #selector(NSResponder.cancelOperation(_:)) {

            // Close editor if need
            if state == .collapse {
                autoCompleteDelegate?.autoCompleteShouldCloseEditor(self)
            }

            // Close suggestion
            closeSuggestion()

            return true
        }

        // Down key
        if commandSelector == #selector(NSResponder.moveDown(_:)) {
            if state == .collapse {
                state = .expand
                autoCompleteView.filter(with: "")
                if window?.firstResponder != self.currentEditor() {
                    window?.makeFirstResponder(self)
                }
            }
            return autoCompleteView.tableView.handleKeyboardEvent(currentEvent)
        }

        // Up key
        if commandSelector == #selector(NSResponder.moveUp(_:)) {
            return autoCompleteView.tableView.handleKeyboardEvent(currentEvent)
        }

        // Tab
        if commandSelector == #selector(NSResponder.insertTab(_:)) {
            return autoCompleteView.tableView.handleKeyboardEvent(currentEvent)
        }

        return false
    }
}
