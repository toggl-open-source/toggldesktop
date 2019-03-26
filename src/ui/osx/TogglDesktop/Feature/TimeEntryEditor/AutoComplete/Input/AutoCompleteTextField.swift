//
//  AutoCompleteTextField.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 3/25/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

final class AutoCompleteTextField: NSTextField, NSTextFieldDelegate {

    // MARK: OUTLET

    private lazy var arrowBtn: CursorButton = {
        let button = CursorButton(frame: NSRect.zero)
        button.image = NSImage(named: NSImage.Name("arrow-section-close"))
        button.cursor = .pointingHand
        return button
    }()

    // MARK: Variables

    private lazy var autoCompleteWindow = AutoCompleteViewWindow()

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
        autoCompleteWindow.prepare(with: dataSource)
    }

    func controlTextDidBeginEditing(_ obj: Notification) {
        
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

    private func presentAutoComplete() {
        arrowBtn.image = NSImage(named: NSImage.Name("arrow-section-open"))

        // Layout frame and position
        autoCompleteWindow.layout(with: self)

        // Present if need
        if !autoCompleteWindow.isVisible {
            window?.addChildWindow(autoCompleteWindow,
                                   ordered: .above)
            autoCompleteWindow.makeKey()
        }

        // Filter
        autoCompleteWindow.filter(with: self.stringValue)
    }
}

// MARK: Private

extension AutoCompleteTextField {

    fileprivate func initCommon() {
        delegate = self
    }

    fileprivate func initArrowBtn() {
        arrowBtn.translatesAutoresizingMaskIntoConstraints = false
        addSubview(arrowBtn)
        arrowBtn.centerYAnchor.constraint(equalTo: centerYAnchor, constant: 0).isActive = true
        arrowBtn.trailingAnchor.constraint(equalTo: trailingAnchor, constant: -15.0).isActive = true
    }
}
