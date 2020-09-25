//
//  TimerDescriptionFieldHandler.swift
//  TogglDesktop
//
//  Created by Andrew Nester on 23.09.2020.
//  Copyright © 2020 Toggl. All rights reserved.
//

import Foundation

class TimerDescriptionFieldHandler {

    private enum Constants {
        static let projectToken: Character = "@"
        static let tagToken: Character = "#"
    }

    enum State {
        case descriptionUpdate(String)
        case projectDropdownShow
        case projectFilter(String)
        case autocompleteFilter(String)
    }

    var state: State = .descriptionUpdate("") {
        didSet {
            onStateChanged?(state)
        }
    }

    var onStateChanged: ((State) -> Void)?

    func textFieldTextDidChange(_ textField: NSTextField) {
        let editor = textField.currentEditor()!

        let text = editor.string
        let cursorLocation = editor.selectedRange.location

        let (token, query) = text.findTokenAndQueryMatchesForAutocomplete([Constants.projectToken], cursorLocation)

        print(">>> token = \(String(describing: token)); query = \(query)")

        switch (token, query) {
        case (Constants.projectToken, ""):
            state = .projectDropdownShow
        case (Constants.projectToken, _):
            state = .projectFilter(query)
        case (nil, query) where query.count > 2:
            state = .autocompleteFilter(query)
        default:
            state = .descriptionUpdate(text)
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
