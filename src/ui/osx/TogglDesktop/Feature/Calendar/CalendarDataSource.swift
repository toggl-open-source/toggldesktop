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
        static let shiftWeek = 4 * 4 // 4 months
        static let cellID = NSUserInterfaceItemIdentifier("DateCellViewItem")
        static let cellNibName = NSNib.Name("DateCellViewItem")
    }

    // MARK: Variables

    weak var delegate: CalendarDataSourceDelegate?
    private var selectedData: DateInfo?
    private var calendar: [DateInfo] = []
    var indexForCurrentDate: Int {
        return calendar.count / 2
    }

    // MARK: Init

    func render(at date: Date) {
        selectedData = DateInfo(date: date)
        calendar = calculateDates(from: date)
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

        let shortMonthSymbols = Calendar.current.shortMonthSymbols
        let infos = calendar.map { (data) -> [DateInfo] in
            return data.days.map { day -> DateInfo in
                return DateInfo(day: day, month: data.month, monthTitle: shortMonthSymbols[data.month - 1], year: data.year)
            }
        }

        var items: [DateInfo] = []
        for info in infos {
            items.append(contentsOf: info)
        }

        return items
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
        return calendar.count
    }

    func collectionView(_ collectionView: NSCollectionView, itemForRepresentedObjectAt indexPath: IndexPath) -> NSCollectionViewItem {
        guard let view = collectionView.makeItem(withIdentifier: Constants.cellID, for: indexPath) as? DateCellViewItem else { return NSCollectionViewItem() }
        let info = calendar[indexPath.item]
        let isCurrentDate = info.isSameDay(with: selectedData!)
        let isCurrentMonth = info.month == selectedData!.month
        view.render(with: info, highlight: isCurrentDate, isCurrentMonth: isCurrentMonth)
        return view
    }

    func collectionView(_ collectionView: NSCollectionView, didSelectItemsAt indexPaths: Set<IndexPath>) {
        guard let selectedIndex = indexPaths.first,
            let date = calendar[safe: selectedIndex.item]
            else { return }
        delegate?.calendarDidSelect(buildDate(day: date.day, month: date.month, year: date.year))
    }
}
