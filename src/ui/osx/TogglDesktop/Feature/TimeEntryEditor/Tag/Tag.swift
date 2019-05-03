//
//  Tag.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 4/5/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

final class Tag {

    static let noMatching = Tag(name: "", isEmptyTag: true)
    static let moreTag = Tag(name: "...", isMoreTag: true)

    let name:String
    let isEmptyTag: Bool
    let isMoreTag: Bool

    init(viewItem: ViewItem) {
        self.name = viewItem.name ?? ""
        self.isEmptyTag = false
        self.isMoreTag = false
    }

    init(name: String, isEmptyTag: Bool = false, isMoreTag: Bool = false) {
        self.name = name
        self.isEmptyTag = isEmptyTag
        self.isMoreTag = isMoreTag
    }
}

extension Array where Element == Tag {

    func toNames() -> [String] {
        return map { $0.name }
    }
}
