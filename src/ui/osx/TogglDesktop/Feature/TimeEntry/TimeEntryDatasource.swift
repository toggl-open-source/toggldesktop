//
//  TimeEntryDatasource.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 2/20/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

class TimeEntryHeader {

    let date: String
    let totalTime: String

    init(date: String, totalTime: String) {
        self.date = date
        self.totalTime = totalTime
    }
}

class TimeEntrySection {

    let header: TimeEntryHeader
    let entries: [TimeEntryViewItem]
    let isLoadMore: Bool

    init(header: TimeEntryHeader, entries: [TimeEntryViewItem], isLoadMore: Bool = false) {
        self.header = header
        self.entries = entries
        self.isLoadMore = isLoadMore
    }

    class func loadMoreSection() -> TimeEntrySection {
        return TimeEntrySection(header: TimeEntryHeader(date: "", totalTime: ""),
                                entries: [],
                                isLoadMore: true)
    }

}

@objcMembers
class TimeEntryDatasource: NSObject {

    // MARK: Variable

    private(set) var sections: [TimeEntrySection]

    // MARK: Init

    init(collectionView: NSCollectionView) {
        self.sections = []

        let flowLayout = NSCollectionViewFlowLayout()
        flowLayout.itemSize = NSSize(width: 280, height: 64)
        flowLayout.sectionInset = NSEdgeInsets(top: 10, left: 10, bottom: 10, right: 10)
        flowLayout.minimumInteritemSpacing = 1000
        flowLayout.minimumLineSpacing = 1000
        flowLayout.scrollDirection = .vertical
        collectionView.collectionViewLayout = flowLayout

        super.init()
    }

    func process(_ timeEntries: [TimeEntryViewItem], showLoadMore: Bool) {
        print("======= Render count \(timeEntries.count)")
        print(timeEntries)
        print("")

        let groups = timeEntries.groupSort(ascending: false, byDate: { $0.started })
        let sections = groups.compactMap { group -> TimeEntrySection? in
            guard let firstEntry = group.first else { return nil }
            let header = TimeEntryHeader(date: firstEntry.formattedDate, totalTime: firstEntry.duration)
            return TimeEntrySection(header: header, entries: group)
        }

        self.sections = sections
    }
}

extension TimeEntryDatasource: NSCollectionViewDataSource, NSCollectionViewDelegate, NSCollectionViewDelegateFlowLayout {

    func numberOfSections(in collectionView: NSCollectionView) -> Int {
        return sections.count
    }

    func collectionView(_ collectionView: NSCollectionView, numberOfItemsInSection section: Int) -> Int {
        let sectionItem = sections[section]
        return sectionItem.entries.count
    }

    func collectionView(_ collectionView: NSCollectionView, itemForRepresentedObjectAt indexPath: IndexPath) -> NSCollectionViewItem {
        guard let cell = collectionView.makeItem(withIdentifier: NSUserInterfaceItemIdentifier("TimeEntryCell"),
                                                 for: indexPath) as? TimeEntryCell else {
            fatalError()
        }
        let section = sections[indexPath.section]
        let item = section.entries[indexPath.item]
        //cell.config(item)

        return cell
    }

    func collectionView(_ collectionView: NSCollectionView, viewForSupplementaryElementOfKind kind: NSCollectionView.SupplementaryElementKind, at indexPath: IndexPath) -> NSView {
        let header = collectionView.makeSupplementaryView(ofKind: NSCollectionView.elementKindSectionHeader,
                                                          withIdentifier: NSUserInterfaceItemIdentifier("HeaderView"),
                                                          for: indexPath) as! TimeHeaderView

        let section = sections[indexPath.section]
        header.config(section.header)

        return header
    }
}

extension Sequence {

    func groupSort(ascending: Bool = true, byDate dateKey: (Iterator.Element) -> Date) -> [[Iterator.Element]] {
        var categories: [[Iterator.Element]] = []
        for element in self {
            let key = dateKey(element)
            guard let dayIndex = categories.index(where: { $0.contains(where: { Calendar.current.isDate(dateKey($0), inSameDayAs: key) }) }) else {
                guard let nextIndex = categories.index(where: { $0.contains(where: { dateKey($0).compare(key) == (ascending ? .orderedDescending : .orderedAscending) }) }) else {
                    categories.append([element])
                    continue
                }
                categories.insert([element], at: nextIndex)
                continue
            }

            guard let nextIndex = categories[dayIndex].index(where: { dateKey($0).compare(key) == (ascending ? .orderedDescending : .orderedAscending) }) else {
                categories[dayIndex].append(element)
                continue
            }
            categories[dayIndex].insert(element, at: nextIndex)
        }
        return categories
    }
}
