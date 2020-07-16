//
//  TagAutoCompleteTextField.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 4/5/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

final class TagAutoCompleteTextField: AutoCompleteTextField, NSWindowDelegate {

    // Hack
    // Because the TagAutoCompleteTextField is inside the AutoCompleteWindow
    // So we have to prevent the action to closeSuggestion when the textField is resigned
    private var skipCloseAutocomplete = false

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

    override func controlTextDidEndEditing(_ obj: Notification) {
        if !skipCloseAutocomplete {
            super.controlTextDidEndEditing(obj)
        }
    }

    override func didPresentAutoComplete() {
        isHidden = false
        removeFromSuperview()

        // Add
        autoCompleteView.placeholderBox.isHidden = false
        translatesAutoresizingMaskIntoConstraints = false
        autoCompleteView.placeholderBoxContainerView.addSubview(self)
        edgesToSuperView()

        DispatchQueue.main.async {
            self.autoCompleteWindow.makeKeyAndOrderFront(nil)
            self.autoCompleteWindow.makeFirstResponder(self)

            // We don't select all text
            // so user can start typying
            self.currentEditor()?.moveToEndOfDocument(nil)
        }
    }

    func windowDidResignMain(_ notification: Notification) {
        closeSuggestion()
    }

    func focus() {
        // If we makeFirstResponder, it will trigger the -controlTextDidEndEditing and collapse the AutoCompleteView
        skipCloseAutocomplete = true
        window?.makeFirstResponder(self)
        skipCloseAutocomplete = false
    }

    override func control(_ control: NSControl, textView: NSTextView, doCommandBy commandSelector: Selector) -> Bool {
        if commandSelector == #selector(NSResponder.insertTab(_:)) {
            skipCloseAutocomplete = true
        }
        let handled = super.control(control, textView: textView, doCommandBy: commandSelector)
        skipCloseAutocomplete = false
        return handled
    }
}
