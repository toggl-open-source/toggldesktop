//
//  Tag.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 4/5/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

final class Tag {

    let name:String

    init(viewItem: ViewItem) {
        self.name = viewItem.name ?? ""
    }

    init(name: String) {
        self.name = name
    }
}
