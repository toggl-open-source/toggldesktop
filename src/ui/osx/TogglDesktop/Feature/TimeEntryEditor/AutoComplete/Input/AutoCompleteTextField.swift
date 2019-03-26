//
//  AutoCompleteTextField.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 3/25/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

final class AutoCompleteTextField: NSTextField, NSTextFieldDelegate {

    // MARK: Variables
    private lazy var autoCompleteWindow = AutoCompleteViewWindow()

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

    func prepare(with dataSource: AutoCompleteViewDataSource, parentView: NSView) {
        autoCompleteWindow.prepare(with: dataSource)
    }

    func controlTextDidBeginEditing(_ obj: Notification) {

    }

    func controlTextDidEndEditing(_ obj: Notification) {
        autoCompleteWindow.cancel()
    }

    func controlTextDidChange(_ obj: Notification) {

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
}
