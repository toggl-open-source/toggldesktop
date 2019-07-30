//
//  Array+ByGroup.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 2/21/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

extension Sequence {

    func groupSort(ascending: Bool = true, byDate dateKey: (Iterator.Element) -> Date) -> [[Iterator.Element]] {
        var categories: [[Iterator.Element]] = []
        for element in self {
            let key = dateKey(element)
            guard let dayIndex = categories.firstIndex(where: { $0.contains(where: { Calendar.current.isDate(dateKey($0), inSameDayAs: key) }) }) else {
                guard let nextIndex = categories.firstIndex(where: { $0.contains(where: { dateKey($0).compare(key) == (ascending ? .orderedDescending : .orderedAscending) }) }) else {
                    categories.append([element])
                    continue
                }
                categories.insert([element], at: nextIndex)
                continue
            }

            // Add the rest element to list without sorting
            categories[dayIndex].append(element)
        }
        return categories
    }
}
