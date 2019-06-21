//
//  TimelineDatasource.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 6/21/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

class TimelineDatasource: NSObject {

    // MARK: Variables

    private unowned let collectionView: NSCollectionView
    private var timeline: TimelineData?

    // MARK: Init

    init(_ collectionView: NSCollectionView) {
        self.collectionView = collectionView
        super.init()
        collectionView.delegate = self
        collectionView.dataSource = self
        collectionView.collectionViewLayout = TimelineFlowLayout()
    }

    func render(_ timeline: TimelineData) {
        self.timeline = timeline
        collectionView.reloadData()
    }
}

extension TimelineDatasource: NSCollectionViewDataSource, NSCollectionViewDelegateFlowLayout {

    func numberOfSections(in collectionView: NSCollectionView) -> Int {
        return 3
    }

    func collectionView(_ collectionView: NSCollectionView, numberOfItemsInSection section: Int) -> Int {
        return 0
    }

    func collectionView(_ collectionView: NSCollectionView, itemForRepresentedObjectAt indexPath: IndexPath) -> NSCollectionViewItem {
        return NSCollectionViewItem()
    }
}
