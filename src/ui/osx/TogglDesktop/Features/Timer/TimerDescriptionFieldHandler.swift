//
//  TimerDescriptionFieldHandler.swift
//  TogglDesktop
//
//  Created by Andrew Nester on 23.09.2020.
//  Copyright © 2020 Toggl. All rights reserved.
//

import Foundation

class TimerDescriptionFieldHandler {

    enum Constants {
        static let projectToken: Character = "@"
        static let tagToken: Character = "#"
    }

    enum State {
        case descriptionUpdate(String)
        case projectDropdownShow
        case projectFilter(String)
        case autocompleteShow
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

        print(">>> token = \(token); query = \(query)")

        return

        let lastTypedIndex = text.index(text.startIndex, offsetBy: cursorLocation - 1)
        let typedSymbol = text[lastTypedIndex]

        print("<<< typedSymbol = \(typedSymbol)")

        if typedSymbol == "@" {
            var canPresentedDropdown = false
            if cursorLocation == 1 {
                canPresentedDropdown = true
            } else {
                let lastTwoSymbols = text[text.index(before: lastTypedIndex)...lastTypedIndex]
                if String(lastTwoSymbols) == " @" {
                    print(">>>>>> showing project dropdown >>>>>>")
                    canPresentedDropdown = true
                }
            }
            if canPresentedDropdown {
                state = .projectDropdownShow
            }
        } else if case .projectDropdownShow = state {
            let beforeCursor = text[text.startIndex...lastTypedIndex]
            if let shortcutIndex = beforeCursor.lastIndex(of: "@") {
                let searchText = text[text.index(after: shortcutIndex)...lastTypedIndex]
                print(">>>>>>>>> filtering with: \(searchText)")
                state = .projectFilter(String(searchText))
            }
        } else {
            state = .descriptionUpdate(text)
        }

//        if cursorLocation > 0 {
//            var canPresentedDropdown = false
//
//            if cursorLocation == 1 {
//                if typedSymbol == "@" {
//                    print(">>>>>> showing project dropdown >>>>>>")
//                    canPresentedDropdown = true
//                }
//            } else {
//                let lastTwoSymbols = text[text.index(before: lastTypedIndex)...lastTypedIndex]
//                if String(lastTwoSymbols) == " @" {
//                    print(">>>>>> showing project dropdown >>>>>>")
//                    canPresentedDropdown = true
//                }
//            }
//
//            if canPresentedDropdown {
//                state = .projectDropdownShow
//            } else if case .projectDropdownShow = state {
//                let beforeCursor = text[text.startIndex...lastTypedIndex]
//                if let shortcutIndex = beforeCursor.lastIndex(of: "@") {
//                    let searchText = text[text.index(after: shortcutIndex)...lastTypedIndex]
//                    print(">>>>>>>>> filtering with: \(searchText)")
//                    state = .projectFilter(String(searchText))
//                }
//            }
//        }
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
