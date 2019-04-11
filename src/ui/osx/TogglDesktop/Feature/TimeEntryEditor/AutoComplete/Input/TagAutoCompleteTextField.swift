//
//  TagAutoCompleteTextField.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 4/5/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

final class TagAutoCompleteTextField: AutoCompleteTextField, NSWindowDelegate {

    private var isMovedToWindowsView = false

    // The design for TagTextField is different than other
    // The AutoCompleteWindow will contain the TextView
    override var isSeperateWindow: Bool {
        return false
    }

    override init(frame frameRect: NSRect) {
        super.init(frame: frameRect)
        autoCompleteWindow.delegate = self
    }

    override func awakeFromNib() {
        super.awakeFromNib()
        autoCompleteWindow.delegate = self
    }

    required init?(coder: NSCoder) {
        super.init(coder: coder)
    }

    override func didPresentAutoComplete() {
        guard !isMovedToWindowsView else { return }
        isMovedToWindowsView = true
        isHidden = false
        removeFromSuperview()

        autoCompleteView.placeholderBox.isHidden = false

        translatesAutoresizingMaskIntoConstraints = false
        autoCompleteView.placeholderBoxContainerView.addSubview(self)
        edgesToSuperView()
    }

    func windowDidResignMain(_ notification: Notification) {
        closeSuggestion()
    }
}
