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
}
