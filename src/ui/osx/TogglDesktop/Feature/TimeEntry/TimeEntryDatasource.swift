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
class TimeEntryDatasource: NSObject, NSCollectionViewDataSource, NSCollectionViewDelegate, NSCollectionViewDelegateFlowLayout {

    // MARK: Variable

    private(set) var sections: [TimeEntrySection]
    private let collectionView: NSCollectionView

    // MARK: Init

    init(collectionView: NSCollectionView) {
        self.sections = []

        let flowLayout = NSCollectionViewFlowLayout()
        flowLayout.itemSize = NSSize(width: 280, height: 64)
        flowLayout.sectionInset = NSEdgeInsets(top: 10, left: 10, bottom: 10, right: 10)
        flowLayout.minimumInteritemSpacing = 0
        flowLayout.minimumLineSpacing = 0
        flowLayout.scrollDirection = .vertical
        flowLayout.headerReferenceSize = CGSize(width: 280, height: 36)
        collectionView.collectionViewLayout = flowLayout
        collectionView.register(NSNib(nibNamed: NSNib.Name("TimeEntryCell"), bundle: nil),
                                forItemWithIdentifier: NSUserInterfaceItemIdentifier("TimeEntryCell"))
        collectionView.register(NSNib(nibNamed: NSNib.Name("TimeHeaderView"), bundle: nil),
                                forSupplementaryViewOfKind: NSCollectionView.elementKindSectionHeader,
                                withIdentifier: NSUserInterfaceItemIdentifier("TimeHeaderView"))
        self.collectionView = collectionView
        collectionView.wantsLayer = true
        if #available(OSX 10.13, *) {
            collectionView.backgroundColors = [NSColor(named: NSColor.Name("collectionview-background-color"))!]
        } else {
            // Fallback on earlier versions
        }
        super.init()
        collectionView.delegate = self
        collectionView.dataSource = self
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
        self.collectionView.reloadData()
        DispatchQueue.main.asyncAfter(deadline: DispatchTime.now() + .seconds(1)) {
            self.collectionView.reloadData()
        }

        print("Reload section \(sections.count)")
    }
}

extension TimeEntryDatasource {

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

        // Render data
        cell.render(item)

        // Get shadow mode by index
        let mode: TimeEntryCell.ShadowMode
        if indexPath.item == 0 {
            mode = .top
        } else if indexPath.item == (section.entries.count - 1) {
            mode = .bottom
        } else {
            mode = .middle
        }

        cell.drawShadow(with: mode)

        return cell
    }

    func collectionView(_ collectionView: NSCollectionView, viewForSupplementaryElementOfKind kind: NSCollectionView.SupplementaryElementKind, at indexPath: IndexPath) -> NSView {
        let header = collectionView.makeSupplementaryView(ofKind: NSCollectionView.elementKindSectionHeader,
                                                          withIdentifier: NSUserInterfaceItemIdentifier("TimeHeaderView"),
                                                          for: indexPath) as! TimeHeaderView

        let section = sections[indexPath.section]
        header.config(section.header)
        return header
    }

    func collectionView(_ collectionView: NSCollectionView, layout collectionViewLayout: NSCollectionViewLayout, referenceSizeForHeaderInSection section: Int) -> NSSize {

        return CGSize(width: collectionView.frame.size.width - 20.0, height: 36)
    }

    func collectionView(_ collectionView: NSCollectionView, layout collectionViewLayout: NSCollectionViewLayout, sizeForItemAt indexPath: IndexPath) -> NSSize {
        return CGSize(width: collectionView.frame.size.width - 20.0, height: 64)
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
