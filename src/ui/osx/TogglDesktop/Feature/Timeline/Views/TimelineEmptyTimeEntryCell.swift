//
//  TimelineEmptyTimeEntryCell.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 7/2/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

class TimelineEmptyTimeEntryCell: NSCollectionViewItem {

    // MARK: OUTLET

    private var topOffset: NSLayoutConstraint!
    private var bottomOffset: NSLayoutConstraint!
    
    // MARK: Variables

    private lazy var dashedBorderView = TimelineDashedCornerView()

    // MARK: View

    override func viewDidLoad() {
        super.viewDidLoad()

        initDashedView()
    }

    func config(for timeEntry: TimelineBaseTimeEntry, at zoomLevel: TimelineDatasource.ZoomLevel) {
        var gap = zoomLevel.minimumGap
        if (gap * 2.0) >= view.frame.height {
            gap = 1.0
        }
        topOffset.constant = gap
        bottomOffset.constant = gap * -1.0

        // If the size is too smal
        // It's better to reduce the corner radius
        if view.frame.height <= 20.0 {
            dashedBorderView.cornerRadius = 4
        }
        dashedBorderView.setNeedsDisplay(dashedBorderView.bounds)
        dashedBorderView.displayIfNeeded()
    }
}

extension TimelineEmptyTimeEntryCell {

    fileprivate func initDashedView() {
        view.addSubview(dashedBorderView)
        dashedBorderView.translatesAutoresizingMaskIntoConstraints = false
        dashedBorderView.leadingAnchor.constraint(equalTo: view.leadingAnchor, constant: 0).isActive = true
        dashedBorderView.trailingAnchor.constraint(equalTo: view.trailingAnchor, constant: 0).isActive = true
        topOffset = dashedBorderView.topAnchor.constraint(equalTo: view.topAnchor, constant: 0)
        bottomOffset = dashedBorderView.bottomAnchor.constraint(equalTo: view.bottomAnchor, constant: 0)
        topOffset.isActive = true
        bottomOffset.isActive = true
    }
}
