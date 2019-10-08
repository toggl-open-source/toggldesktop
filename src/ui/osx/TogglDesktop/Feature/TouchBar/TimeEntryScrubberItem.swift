//
//  TimeEntryScrubberItem.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 10/8/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

@available(OSX 10.12.2, *)
final class TimeEntryScrubberItem: NSScrubberTextItemView {

    // MARK: Public

    func config(_ item: TimeEntryViewItem) {
        title = item.touchBarTitle
    }
}
