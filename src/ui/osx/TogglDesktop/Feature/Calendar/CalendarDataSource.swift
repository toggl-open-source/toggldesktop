//
//  CalendarDataSource.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 4/19/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Foundation

protocol CalendarDataSourceDelegate: class {

    func calendarDidSelect(_ date: Date)
}

final class CalendarDataSource: NSObject {

    struct Constants {

        static let shiftWeek = 6 * 4 // 6 months
        static let cellID = NSUserInterfaceItemIdentifier("DateCellViewItem")
        static let cellNibName = NSNib.Name("DateCellViewItem")
    }

    // MARK: Variables

    weak var delegate: CalendarDataSourceDelegate?
    var selectedDate: Date {
        didSet {
            let result = CalendarDataSource.calculateDateRange(with: selectedDate)
            currentDate = result.0
            fromDate = result.1
            toDate = result.2
            numberOfDay = fromDate.date.daysBetween(endDate: toDate.date)
            indexForCurrentDate = numberOfDay / 2
        }
    }
    private var currentDate: DateInfo!
    private var fromDate: DateInfo!
    private var toDate: DateInfo!
    private var numberOfDay: Int!
    private(set) var indexForCurrentDate = 0

    // MARK: Init

    init(selectedDate: Date) {
        self.selectedDate = selectedDate
    }

    private class func calculateDateRange(with selectedDate: Date) -> (DateInfo, DateInfo, DateInfo) {
        let currentDate = DateInfo(date: selectedDate.toLocalTime())

        // firstDayOfWeek will return the Sunday
        // But we need monday -> advance by 2
        let firstDayOfWeek = selectedDate.firstDayOfWeek()?.nextDate() ?? selectedDate
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
        return numberOfDay
    }

    func collectionView(_ collectionView: NSCollectionView, itemForRepresentedObjectAt indexPath: IndexPath) -> NSCollectionViewItem {
        guard let view = collectionView.makeItem(withIdentifier: Constants.cellID, for: indexPath) as? DateCellViewItem,
            let date = Calendar.current.date(byAdding: .day, value: indexPath.item, to: fromDate.date) else { return NSCollectionViewItem() }
        let info = DateInfo(date: date)
        let isCurrentDate = info.isSameDay(with: currentDate)
        view.render(with: info, highlight: isCurrentDate)
        return view
    }

    func collectionView(_ collectionView: NSCollectionView, didSelectItemsAt indexPaths: Set<IndexPath>) {
        guard let selectedIndex = indexPaths.first,
            let date = Calendar.current.date(byAdding: .day, value: selectedIndex.item, to: fromDate.date)
            else { return }
        delegate?.calendarDidSelect(date)
    }
}
