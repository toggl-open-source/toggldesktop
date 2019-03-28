//
//  AutoCompleteTextField.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 3/25/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

class AutoCompleteTextField: NSTextField, NSTextFieldDelegate, AutoCompleteViewDelegate {

    // MARK: OUTLET

    private lazy var arrowBtn: CursorButton = {
        let button = CursorButton(frame: NSRect.zero)
        button.image = NSImage(named: NSImage.Name("arrow-section-close"))
        button.cursor = .pointingHand
        return button
    }()

    // MARK: Variables

    lazy var autoCompleteWindow: AutoCompleteViewWindow = {
        let window = AutoCompleteViewWindow(view: autoCompleteView)
        return window
    }()
    private let autoCompleteView: AutoCompleteView = AutoCompleteView.xibView()

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

    func prepare(with dataSource: AutoCompleteViewDataSource, parentView: NSView) {
        autoCompleteView.prepare(with: dataSource)
    }

    func controlTextDidEndEditing(_ obj: Notification) {
        arrowBtn.image = NSImage(named: NSImage.Name("arrow-section-close"))
        autoCompleteWindow.cancel()
    }

    func controlTextDidChange(_ obj: Notification) {
        presentAutoComplete()
    }

    @objc func arrowBtnOnTap() {
        presentAutoComplete()
    }

    func closeSuggestion() {
        autoCompleteWindow.cancel()
    }

    func updateWindowContent(with view: NSView, height: CGFloat) {
        autoCompleteWindow.contentView = view
        autoCompleteWindow.layoutFrame(with: self, height: height)
        autoCompleteWindow.makeKey()
    }

    private func presentAutoComplete() {

        //
        arrowBtn.image = NSImage(named: NSImage.Name("arrow-section-open"))

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
