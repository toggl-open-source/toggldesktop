//
//  TimerDescriptionFieldHandler.swift
//  TogglDesktop
//
//  Created by Andrew Nester on 23.09.2020.
//  Copyright © 2020 Toggl. All rights reserved.
//

import Foundation

class TimerDescriptionFieldHandler: NSResponder, NSTextFieldDelegate {

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

    enum Action {
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

    func didCloseProjectDropdown() {
        state = .descriptionUpdate
    }

    // MARK: NSTextFieldDelegate

    func controlTextDidChange(_ obj: Notification) {
        guard let textField = obj.object as? NSTextField else { return }

        let editor = textField.currentEditor()!

        let text = editor.string
        let cursorLocation = editor.selectedRange.location

        let (token, query) = text.findTokenAndQueryMatchesForAutocomplete([Constants.projectToken], cursorLocation)

        print(">>> token = \(String(describing: token)); query = \(query)")

        switch (token, query) {
        case (Constants.projectToken, _):
            state = .projectFilter(query)
        case (nil, query) where query.count >= Constants.autoCompleteMinFilterLength:
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
        // TODO: discuss if we need this
        // if yes, then `autoCompleteMinFilterLength` seems to conflict with it
//        if commandSelector == #selector(moveDown(_:)) {
//            viewModel.filterAutocomplete(with: descriptionTextField.stringValue)
//            descriptionTextField.toggleList(true)
//            return true
//        }
        if commandSelector == #selector(insertNewline(_:)) {
            _ = onPerformAction(.unfocus)
            _ = onPerformAction(.startTimeEntry)
            state = .descriptionUpdate
            return true
        }
        return false
    }
}

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
