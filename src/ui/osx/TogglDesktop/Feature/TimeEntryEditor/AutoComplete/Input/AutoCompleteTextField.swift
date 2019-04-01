//
//  AutoCompleteTextField.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 3/25/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

class AutoCompleteTextField: NSTextField, NSTextFieldDelegate, AutoCompleteViewDelegate {

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

    lazy var autoCompleteWindow: AutoCompleteViewWindow = AutoCompleteViewWindow(view: autoCompleteView)
    lazy var autoCompleteView: AutoCompleteView = AutoCompleteView.xibView()
    private var _state = State.collapse {
        didSet {
            let name = state == .collapse ? NSImage.Name("arrow-section-open") : NSImage.Name("arrow-section-close")
            arrowBtn.image = NSImage(named: name)

            switch state {
            case .collapse:
                closeAutoComplete()
            case .expand:
                presentAutoComplete()
            }
        }
    }
    private var state: State {
        get { return _state }
        set {
            guard newValue != state else { return }
            _state = newValue
        }
    }

    // MARK: Init

    override init(frame frameRect: NSRect) {
        super.init(frame: frameRect)
        initCommon()
        initArrowBtn()
    }

    required init?(coder: NSCoder) {
        super.init(coder: coder)
        initCommon()
        initArrowBtn()
    }

    // MARK: Public

    func controlTextDidEndEditing(_ obj: Notification) {
        state = .collapse
    }

    func controlTextDidChange(_ obj: Notification) {
        state = .expand
        autoCompleteView.filter(with: self.stringValue)
    }

    @objc func arrowBtnOnTap() {
        switch state {
        case .collapse:
            state = .expand
        case .expand:
            state = .collapse
        }
    }

    func closeSuggestion() {
        state = .collapse
    }

    func updateWindowContent(with view: NSView, height: CGFloat) {
        autoCompleteWindow.contentView = view
        autoCompleteWindow.layoutFrame(with: self, height: height)
        autoCompleteWindow.makeKey()
    }

    private func closeAutoComplete() {
        autoCompleteWindow.cancel()
    }

    private func presentAutoComplete() {

        // Set auto complete table
        autoCompleteWindow.contentView = autoCompleteView
        autoCompleteWindow.setContentSize(autoCompleteView.frame.size)
        
        // Layout frame and position
        autoCompleteWindow.layoutFrame(with: self, height: autoCompleteView.frame.size.height)

        // Present if need
        if !autoCompleteWindow.isVisible {
            window?.addChildWindow(autoCompleteWindow,
                                   ordered: .above)
        }

        // Filter
        autoCompleteView.filter(with: self.stringValue)
    }

    func didTapOnCreateButton() {

    }
}

// MARK: Private

extension AutoCompleteTextField {

    fileprivate func initCommon() {
        delegate = self
        autoCompleteView.delegate = self
        state = .collapse
    }

    fileprivate func initArrowBtn() {
        arrowBtn.translatesAutoresizingMaskIntoConstraints = false
        addSubview(arrowBtn)
        arrowBtn.centerYAnchor.constraint(equalTo: centerYAnchor, constant: 0).isActive = true
        arrowBtn.trailingAnchor.constraint(equalTo: trailingAnchor, constant: -15.0).isActive = true
    }

    func control(_ control: NSControl, textView: NSTextView, doCommandBy commandSelector: Selector) -> Bool {
        guard let currentEvent = NSApp.currentEvent else {
            return false
        }

        // Enter
        if commandSelector == #selector(NSResponder.insertNewline(_:)) {
            autoCompleteView.tableView.keyDown(with: currentEvent)
            return true
        }

        // Escape
        if commandSelector == #selector(NSResponder.cancelOperation(_:)) {
            return true
        }

        // Down key
        if commandSelector == #selector(NSResponder.moveDown(_:)) {
            autoCompleteView.tableView.keyDown(with: currentEvent)
            return true
        }

        // Down key
        if commandSelector == #selector(NSResponder.moveUp(_:)) {
            autoCompleteView.tableView.keyDown(with: currentEvent)
            return true
        }

        return false
    }
}
