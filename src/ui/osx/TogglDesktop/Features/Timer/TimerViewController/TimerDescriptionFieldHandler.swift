//
//  TimerDescriptionFieldHandler.swift
//  TogglDesktop
//
//  Created by Andrew Nester on 23.09.2020.
//  Copyright © 2020 Toggl. All rights reserved.
//

import Foundation

class TimerDescriptionFieldHandler: NSResponder {

    private enum Constants {
        static let projectToken: Character = "@"
        static let tagToken: Character = "#"

        static let autoCompleteMinFilterLength = 2
    }

    enum State: Equatable {
        case descriptionUpdate
        case projectFilter(String)
        case autocompleteFilter(String)
    }

    private(set) var state: State = .descriptionUpdate {
        didSet {
            onStateChanged?(state, oldValue)
        }
    }

    /// Called on every `state` change with `newState, oldState` parameters
    var onStateChanged: ((State, State) -> Void)?

    enum Action: Equatable {
        case endEditing
        case startTimeEntry
        case unfocus

        case projectAutoCompleteTableHandleEvent(NSEvent)

        case autoCompleteTableSelectNext
        case autoCompleteTableSelectPrevious
        case autoCompleteSelectCurrent
    }

    /// Called when description field requests an action to be performed.
    /// Closure must return `true` if action was handled.
    var onPerformAction: (Action) -> Bool = { _ in return false }

    private let textField: AutoCompleteInput
    private var kvoToken: NSKeyValueObservation?

    /// Set to `true` to enable Project (@) shortcut feature
    var isProjectShortcutEnabled: Bool

    /// Initializes text field handler.
    /// - Parameters:
    ///   - textField: Text field that will be sending event to this handler
    ///   - enableProjectShortcut: Pass `true` to enable project (@) shortcut feature
    init(textField: AutoCompleteInput, enableProjectShortcut: Bool = false) {
        self.textField = textField
        self.isProjectShortcutEnabled = enableProjectShortcut
        super.init()
        self.textField.delegate = self
        observeTextField()
    }

    required init?(coder: NSCoder) {
        fatalError("init(coder:) has not been implemented")
    }

    func didCloseProjectDropdown() {
        if case .projectFilter = state {
            state = .descriptionUpdate
        }
    }

    func didSelectProject() {
        if case .projectFilter = state {
            removeShortcutQuery(for: Constants.projectToken)
        }
    }

    // MARK: - Private

    /// Removes from text field the text related to `shortcut`
    private func removeShortcutQuery(for shortcut: Character) {
        guard let editor = textField.currentEditor() else { return }

        var text = editor.string
        let lastTypedIndex = text.index(text.startIndex, offsetBy: editor.selectedRange.location - 1)
        let rangeBeforeCursor = text[text.startIndex...lastTypedIndex]
        if let shortcutIndex = rangeBeforeCursor.lastIndex(of: shortcut) {
            let shortcutLocation = text.distance(from: text.startIndex, to: shortcutIndex)

            text.removeSubrange(shortcutIndex...lastTypedIndex)
            editor.string = text

            // moving cursor to the previous shortcut position
            editor.selectedRange = NSRange(location: shortcutLocation, length: 0)

            state = .descriptionUpdate
        }
    }

    private func observeTextField() {
        kvoToken = textField.observe(\.isListHidden) { [weak self] textField, _ in
            guard let self = self else { return }
            // changin state to default if it was .autocompleteFilter before
            // and list was closed, e.g. by clicking on dimming background
            if case .autocompleteFilter = self.state, textField.isListHidden {
                self.state = .descriptionUpdate
            }
        }
    }
}

// MARK: - NSTextFieldDelegate

extension TimerDescriptionFieldHandler: NSTextFieldDelegate {

    func controlTextDidChange(_ obj: Notification) {
        guard let editor = textField.currentEditor() else { return }

        let text = editor.string
        let cursorLocation = editor.selectedRange.location

        let (token, query): (Character?, String)

        if isProjectShortcutEnabled {
            (token, query) = text.findTokenAndQueryMatchesForAutocomplete([Constants.projectToken], cursorLocation)
        } else {
            (token, query) = (nil, text)
        }

        switch (token, query) {
        case (Constants.projectToken, _):
            state = .projectFilter(query)
        case (nil, query):
            state = .autocompleteFilter(query)
        default:
            state = .descriptionUpdate
        }
    }

    func controlTextDidEndEditing(_ obj: Notification) {
        _ = onPerformAction(.endEditing)
        state = .descriptionUpdate
    }

    // MARK: Handling Key Commands

    func control(_ control: NSControl, textView: NSTextView, doCommandBy commandSelector: Selector) -> Bool {
        switch state {
        case .autocompleteFilter:
            return autocompleteControl(doCommandBy: commandSelector)
        case .projectFilter:
            return projectDropdownControl(doCommandBy: commandSelector)
        case .descriptionUpdate:
            return descriptionSimpleControl(doCommandBy: commandSelector)
        }
    }

    private func autocompleteControl(doCommandBy commandSelector: Selector) -> Bool {
        if commandSelector == #selector(moveDown(_:)) {
            return onPerformAction(.autoCompleteTableSelectNext)

        } else if commandSelector == #selector(moveUp(_:)) {
            return onPerformAction(.autoCompleteTableSelectPrevious)

        } else if commandSelector == #selector(insertTab(_:)) {
            defer {
                state = .descriptionUpdate
            }
            return onPerformAction(.autoCompleteSelectCurrent)

        } else if commandSelector == #selector(insertNewline(_:)) {
            var isHandled = false
            isHandled = onPerformAction(.autoCompleteSelectCurrent)
            _ = onPerformAction(.unfocus)
            _ = onPerformAction(.startTimeEntry)
            state = .descriptionUpdate
            return isHandled

        } else if commandSelector == #selector(cancelOperation(_:)) {
            state = .descriptionUpdate
            return true
        }

        return false
    }

    private func projectDropdownControl(doCommandBy commandSelector: Selector) -> Bool {
        guard let currentEvent = NSApp.currentEvent else { return false }

        if commandSelector == #selector(NSResponder.cancelOperation(_:)) {
            state = .descriptionUpdate
            return true
        }

        if commandSelector == #selector(NSResponder.moveDown(_:))
            || commandSelector == #selector(NSResponder.moveUp(_:))
            || commandSelector == #selector(NSResponder.insertTab(_:))
            || commandSelector == #selector(NSResponder.insertNewline(_:)) {
            return onPerformAction(.projectAutoCompleteTableHandleEvent(currentEvent))
        }

        return false
    }

    private func descriptionSimpleControl(doCommandBy commandSelector: Selector) -> Bool {
        if commandSelector == #selector(moveDown(_:)) {
            state = .autocompleteFilter(textField.stringValue)
            textField.toggleList(true)
            return true

        } else if commandSelector == #selector(insertNewline(_:)) {
            _ = onPerformAction(.unfocus)
            _ = onPerformAction(.startTimeEntry)
            state = .descriptionUpdate
            return true
        }
        return false
    }
}

// MARK: - State Extension

extension TimerDescriptionFieldHandler.State {

    func equalCase(to state: Self) -> Bool {
        switch (self, state) {
        case (.descriptionUpdate, .descriptionUpdate):
            return true
        case (.projectFilter, .projectFilter):
            return true
        case (.autocompleteFilter, .autocompleteFilter):
            return true
        default:
            return false
        }
    }
}
