//
//  String+Search.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 4/16/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

extension String {

    // minimal version of fuzzy search
    func fuzzySearch(with word: String) -> Bool {
        let source = self.lowercased().trimmingCharacters(in: CharacterSet.whitespacesAndNewlines)
        let target = word.lowercased().trimmingCharacters(in: CharacterSet.whitespacesAndNewlines)
        let targets = target.components(separatedBy: " ")

        // Normal contain if there is only 1 word
        guard targets.count > 0 else {
            return source.contains(target)
        }

        // Count how many the source contain the target word
        let matchedWord = targets.reduce(0) { (previousResult, target) -> Int in
            if source.contains(target) {
                return previousResult + 1
            }
            return 0
        }

        // If contain all words -> matched
        return matchedWord == targets.count
    }

    func findTokenAndQueryMatchesForAutocomplete(_ token: Character, _ cursorPosition: Int) -> (Character?, String) {
        return findTokenAndQueryMatchesForAutocomplete([token], cursorPosition)
    }

    func findTokenAndQueryMatchesForAutocomplete(_ tokens: [Character], _ cursorPosition: Int) -> (Character?, String) {
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
