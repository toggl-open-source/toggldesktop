//
//  TagAutoCompleteTextField.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 4/5/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

final class TagAutoCompleteTextField: AutoCompleteTextField {

    private var isMovedToWindowsView = false

    
    // The design for TagTextField is different than other
    // The AutoCompleteWindow will contain the TextView
    override var isSeperateWindow: Bool {
        return false
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
}
