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
    private lazy var autoCompleteView: AutoCompleteView = AutoCompleteView.xibView()

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
        autoCompleteView.prepare(with: dataSource)
        layoutAutoCompleteView(with: parentView)
    }

    func controlTextDidBeginEditing(_ obj: Notification) {
        autoCompleteView.isHidden = false
    }

}

// MARK: Private

extension AutoCompleteTextField {

    fileprivate func initCommon() {
        delegate = self
        wantsLayer = true
        layer?.masksToBounds = false
    }

    fileprivate func layoutAutoCompleteView(with parentView: NSView) {
        autoCompleteView.isHidden = true
        autoCompleteView.translatesAutoresizingMaskIntoConstraints = false
        parentView.addSubview(autoCompleteView)

        autoCompleteView.topAnchor.constraint(equalTo: parentView.bottomAnchor, constant: 5).isActive = true
        autoCompleteView.leadingAnchor.constraint(equalTo: parentView.leadingAnchor, constant: 0).isActive = true
        autoCompleteView.trailingAnchor.constraint(equalTo: parentView.trailingAnchor, constant: 0).isActive = true
        autoCompleteView.heightAnchor.constraint(equalToConstant: 200).isActive = true
    }
}
