//
//  CalendarView.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 4/19/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

final class CalendarView: NSView {

    // MARK: OUTLET

    @IBOutlet weak var collectionView: NSCollectionView!

    // MARK: View Cycle

    override func awakeFromNib() {
        super.awakeFromNib()

        initCommon()
        initCollectionView()
    }
}

// MARK: Private

extension CalendarView {

    fileprivate func initCommon() {

    }

    fileprivate func initCollectionView() {
        
    }
}
