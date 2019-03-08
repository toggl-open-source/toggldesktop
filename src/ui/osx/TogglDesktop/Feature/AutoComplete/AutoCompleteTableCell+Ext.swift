//
//  AutoCompleteTableCell+Ext.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 3/8/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

extension AutoCompleteTableCell {

    @objc func cellType(from item: AutocompleteItem) -> AutoCompleteCellType {
        return AutoCompleteCellType(item: item)
    }
}
