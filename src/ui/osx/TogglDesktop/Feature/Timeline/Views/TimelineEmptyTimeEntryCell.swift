//
//  TimelineEmptyTimeEntryCell.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 7/2/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

class TimelineEmptyTimeEntryCell: NSCollectionViewItem {

    // MARK: Variables

    private lazy var dashedBorderView = TimelineDashedCornerView()

    // MARK: View

    override func viewDidLoad() {
        super.viewDidLoad()

        initDashedView()
    }

    func config(for timeEntry: TimelineBaseTimeEntry) {
        
    }
}

extension TimelineEmptyTimeEntryCell {

    fileprivate func initDashedView() {
        view.addSubview(dashedBorderView)
        dashedBorderView.edgesToSuperView()
    }
}
