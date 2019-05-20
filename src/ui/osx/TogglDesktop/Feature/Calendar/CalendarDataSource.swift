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

struct MonthData {
    let days: [Int]
    let month: Int
    let year: Int

    func dropPrefix(at day: Int) -> MonthData {
        let days = Array(day...self.days.count)
        return MonthData(days: days, month: month, year: year)
    }

    func dropSuffix(at day: Int) -> MonthData {
        let days = Array(1...day)
        return MonthData(days: days, month: month, year: year)
    }
}

final class CalendarDataSource: NSObject {

    struct Constants {

        static let shiftWeek = 5 * 4 // 6 months
        static let cellID = NSUserInterfaceItemIdentifier("DateCellViewItem")
        static let cellNibName = NSNib.Name("DateCellViewItem")
    }

    // MARK: Variables

    weak var delegate: CalendarDataSourceDelegate?
    var selectedDate: Date {
        didSet {
            let result = CalendarDataSource.calculateDateRange(with: selectedDate)
            _ = calculateDates(from: selectedDate)
            currentDate = result.0
            fromDate = result.1
            toDate = result.2
//            numberOfDay = fromDate.date.daysBetween(endDate: toDate.date)
            numberOfDay = 0
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
        let currentDate = DateInfo(date: selectedDate)

        // firstDayOfWeek will return the Sunday
        // But we need monday -> advance by 2
        let firstDayOfWeek = selectedDate.firstDayOfWeek() ?? selectedDate
        let from = Calendar.current.date(byAdding: .weekOfYear, value: -Constants.shiftWeek, to: firstDayOfWeek)!
        let to = Calendar.current.date(byAdding: .weekOfYear, value: Constants.shiftWeek, to: firstDayOfWeek)!
        let fromDate = DateInfo(date: from)
        let toDate = DateInfo(date: to)
        return (currentDate, fromDate, toDate)
    }

    private func calculateDates(from selectedDate: Date) -> [DateInfo] {
        let firstDayOfWeek = selectedDate.firstDayOfWeek() ?? selectedDate
        let from = Calendar.current.date(byAdding: .weekOfYear, value: -Constants.shiftWeek, to: firstDayOfWeek)!
        let to = Calendar.current.date(byAdding: .weekOfYear, value: Constants.shiftWeek, to: firstDayOfWeek)!
        let _from = DateInfo(date: from)
        let _to = DateInfo(date: to)

        var currentMonth = _from.month
        var currentYear = _from.year

        var numberOfMonths = from.monthBetween(endDate: to)
        numberOfMonths += 1
        var calendar: [MonthData] = []

        // Convert
        for _ in 0..<numberOfMonths {
            let firstDayInMonth = buildDate(day: 1, month: currentMonth, year: currentYear)
            let totalDateInMonth = Calendar.current.range(of: .day, in: .month, for: firstDayInMonth)!.count
            let days = Array(1...totalDateInMonth)
            let data = MonthData(days: days, month: currentMonth, year: currentYear)
            calendar.append(data)

            // Next
            currentMonth += 1
            if currentMonth > 12 {
                currentMonth = 1
                currentYear += 1
            }
        }

        // Trip out the first and last
        let first = calendar.first!.dropPrefix(at: _from.day)
        let last = calendar.last!.dropSuffix(at: _to.day)
        calendar[0] = first
        calendar[calendar.count - 1] = last

        // Map
        return []
    }

    private func buildDate(day: Int, month: Int, year: Int) -> Date {
        let component = DateComponents(calendar: Calendar.current, year: year, month: month, day: day)
        return Calendar.current.date(from: component)!
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
        return NSCollectionViewItem()
//        guard let view = collectionView.makeItem(withIdentifier: Constants.cellID, for: indexPath) as? DateCellViewItem,
//            let date = Calendar.current.date(byAdding: .day, value: indexPath.item, to: fromDate.date) else { return NSCollectionViewItem() }
//        let info = DateInfo(date: date)
//        let isCurrentDate = info.isSameDay(with: currentDate)
//        let isCurrentMonth = info.month == currentDate.month
//        view.render(with: info, highlight: isCurrentDate, isCurrentMonth: isCurrentMonth)
//        return view
    }

    func collectionView(_ collectionView: NSCollectionView, didSelectItemsAt indexPaths: Set<IndexPath>) {
//        guard let selectedIndex = indexPaths.first,
//            let date = Calendar.current.date(byAdding: .day, value: selectedIndex.item, to: fromDate.date)
//            else { return }
//        delegate?.calendarDidSelect(date)
    }
}
