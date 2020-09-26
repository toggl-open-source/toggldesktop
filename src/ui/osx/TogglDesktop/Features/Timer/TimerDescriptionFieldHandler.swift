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

    var state: State = .descriptionUpdate {
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
    }

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

    private func descriptionSimpleControl(doCommandBy commandSelector: Selector) -> Bool {
        // TODO: discuss if we need this
        // if yes, then `autoCompleteMinFilterLength` seems to conflict with it
//        if commandSelector == #selector(moveDown(_:)) {
//            viewModel.filterAutocomplete(with: descriptionTextField.stringValue)
//            descriptionTextField.toggleList(true)
//            return true
//        }
        return false
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

private extension String {

    func findTokenAndQueryMatchesForAutocomplete(
        _ token: Character,
        _ cursorPosition: Int
    ) -> (Character?, String) {
        return findTokenAndQueryMatchesForAutocomplete([token], cursorPosition)
    }

    func findTokenAndQueryMatchesForAutocomplete(
        _ tokens: [Character],
        _ cursorPosition: Int
    ) -> (Character?, String) {

        do {
            let joinedTokens = tokens.map { String($0) }.joined(separator: "|")
            let regex = try NSRegularExpression(pattern: "(^| )(\(joinedTokens))")
            let searchRange = startIndex..<index(startIndex, offsetBy: cursorPosition.clamp(min: 0, max: self.count))
            let matches = regex.matches(in: self, range: NSRange(searchRange, in: self))

            guard let match = matches.last else { return (nil, self) }

            let queryStart = index(startIndex, offsetBy: match.range.lowerBound)
            let matchSubstring = self[queryStart..<endIndex]
            let matchedTheFirstWord = tokens.contains { matchSubstring.starts(with: String($0)) }
            let queryWithToken = String(matchedTheFirstWord ? matchSubstring : matchSubstring.dropFirst())

            let token = queryWithToken.first
            let query = String(queryWithToken.dropFirst())

            return (token, query)

        } catch {
            return (nil, "")
        }
    }

    static func matches(for regex: String, in text: String) -> [String] {

        do {
            let regex = try NSRegularExpression(pattern: regex)
            let results = regex.matches(in: text,
                                        range: NSRange(text.startIndex..., in: text))
            return results.map {
                String(text[Range($0.range, in: text)!])
            }
        } catch let error {
            print("invalid regex: \(error.localizedDescription)")
            return []
        }
    }
}

private extension Comparable {
    func clamp(min: Self, max: Self) -> Self {
        if self > max {
            return max
        } else if self < min {
            return min
        }

        return self
    }
}
