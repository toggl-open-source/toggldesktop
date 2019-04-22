//
//  CalendarDataSource.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 4/19/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

final class CalendarDataSource: NSObject {

    struct Constants {

        static let shiftWeek = 1 * 4
        static let cellID = NSUserInterfaceItemIdentifier("DateCellViewItem")
        static let cellNibName = NSNib.Name("DateCellViewItem")
    }

    // MARK: Variables

    var selectedDate: Date {
        didSet {
            let result = CalendarDataSource.calculateDateRange(with: selectedDate)
            currentDate = result.0
            fromDate = result.1
            toDate = result.2
        }
    }
    private var currentDate: DateInfo
    private var fromDate: DateInfo
    private var toDate: DateInfo
    private var numberOfItems: Int {
        let count = (toDate.weekOfYear - fromDate.weekOfYear + 1) * 7 // number of week * 7 days a week
        print("number of days \(count)")
        return count
    }

    // MARK: Init

    init(_ selectedDate: Date) {
        self.selectedDate = selectedDate
        let result = CalendarDataSource.calculateDateRange(with: selectedDate)
        currentDate = result.0
        fromDate = result.1
        toDate = result.2
    }

    private class func calculateDateRange(with selectedDate: Date) -> (DateInfo, DateInfo, DateInfo) {
        let currentDate = DateInfo(date: selectedDate)

        // firstDayOfWeek will return the Sunday
        // But we need monday -> advance by 2
        let firstDayOfWeek = selectedDate.firstDayOfWeek()?.nextDate()?.nextDate() ?? selectedDate
        let from = Calendar.current.date(byAdding: .weekOfYear, value: -Constants.shiftWeek, to: firstDayOfWeek)!
        let to = Calendar.current.date(byAdding: .weekOfYear, value: Constants.shiftWeek, to: firstDayOfWeek)!
        let fromDate = DateInfo(date: from)
        let toDate = DateInfo(date: to)
        return (currentDate, fromDate, toDate)
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
        guard let view = collectionView.makeItem(withIdentifier: Constants.cellID, for: indexPath) as? DateCellViewItem else { return NSCollectionViewItem() }
        return view
    }

    func collectionView(_ collectionView: NSCollectionView, didSelectItemsAt indexPaths: Set<IndexPath>) {

    }
}
