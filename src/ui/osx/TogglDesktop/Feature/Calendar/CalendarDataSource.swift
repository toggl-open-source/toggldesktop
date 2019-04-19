//
//  CalendarDataSource.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 4/19/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

final class CalendarDataSource: NSObject {

    private struct Constants {

        static let shiftWeek = 4 * 4
    }

    // MARK: Variables

    var selectedDate: Date
    private var currentDate: DateInfo
    private var fromDate: DateInfo
    private var toDate: DateInfo

    private var numberOfItems: Int {
        return (toDate.weekOfYear - fromDate.weekOfYear + 1) * 7 // number of week * 7 days a week
    }

    // MARK: Init

    init(_ selectedDate: Date) {
        self.selectedDate = selectedDate
        currentDate = DateInfo(date: selectedDate)
        let from = Calendar.current.date(byAdding: .weekOfYear, value: -Constants.shiftWeek, to: selectedDate)!
        let to = Calendar.current.date(byAdding: .weekOfYear, value: Constants.shiftWeek, to: selectedDate)!
        fromDate = DateInfo(date: from)
        toDate = DateInfo(date: to)
    }
}

extension CalendarDataSource: NSCollectionViewDelegate, NSCollectionViewDataSource, NSCollectionViewDelegateFlowLayout {

    func numberOfSections(in collectionView: NSCollectionView) -> Int {
        return 1
    }

    func collectionView(_ collectionView: NSCollectionView, numberOfItemsInSection section: Int) -> Int {
        return numberOfItems
    }

    func collectionView(_ collectionView: NSCollectionView, itemForRepresentedObjectAt indexPath: IndexPath) -> NSCollectionViewItem {
        return NSCollectionViewItem()
    }

    func collectionView(_ collectionView: NSCollectionView, didSelectItemsAt indexPaths: Set<IndexPath>) {

    }
}
