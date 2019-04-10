//
//  Tag.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 4/5/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

final class Tag {

    static let empty = Tag(name: "", isEmptyTag: true)

    let name:String
    let isEmptyTag: Bool

    init(viewItem: ViewItem) {
        self.name = viewItem.name ?? ""
        self.isEmptyTag = false
    }

    init(name: String, isEmptyTag: Bool = false) {
        self.name = name
        self.isEmptyTag = isEmptyTag
    }
}
