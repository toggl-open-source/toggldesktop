//
//  TimeEntryDatasource.swift
//  TogglDesktop
//
//  Created by Nghia Tran on 2/20/19.
//  Copyright © 2019 Alari. All rights reserved.
//

import Cocoa

final class TimeEntryHeader {

    let date: String
    let totalTime: String

    init(date: String, totalTime: String) {
        self.date = date
        self.totalTime = totalTime
    }
}

final class TimeEntrySection {

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
                                entries: [TimeEntryViewItem()],
                                isLoadMore: true)
    }

}

@objcMembers
class TimeEntryDatasource: NSObject {

    // MARK: Variable
    private var firstTime = true
    private(set) var sections: [TimeEntrySection]
    private let collectionView: NSCollectionView
    fileprivate var cellSize: NSSize {
        return CGSize(width: collectionView.frame.size.width - 20.0, height: 64)
    }
    fileprivate var headerSize: NSSize {
        return CGSize(width: collectionView.frame.size.width - 20.0, height: 36)
    }
    fileprivate var loaderMoreSize: NSSize {
        return CGSize(width: collectionView.frame.size.width - 20.0, height: 44)
    }

    // MARK: Init

    init(collectionView: NSCollectionView) {
        self.sections = []

        let flowLayout = VertificalTimeEntryFlowLayout()
        flowLayout.register(NSNib(nibNamed: NSNib.Name("TimeDecoratorView"), bundle: nil),
                            forDecorationViewOfKind: "TimeDecoratorView")
        collectionView.collectionViewLayout = flowLayout
        collectionView.register(NSNib(nibNamed: NSNib.Name("TimeEntryCell"), bundle: nil),
                                forItemWithIdentifier: NSUserInterfaceItemIdentifier("TimeEntryCell"))
        collectionView.register(NSNib(nibNamed: NSNib.Name("LoadMoreCell"), bundle: nil),
                                forItemWithIdentifier: NSUserInterfaceItemIdentifier("LoadMoreCell"))
        collectionView.register(NSNib(nibNamed: NSNib.Name("TimeHeaderView"), bundle: nil),
                                forSupplementaryViewOfKind: NSCollectionView.elementKindSectionHeader,
                                withIdentifier: NSUserInterfaceItemIdentifier("TimeHeaderView"))

        self.collectionView = collectionView
        collectionView.wantsLayer = true
        if #available(OSX 10.13, *) {
            collectionView.backgroundColors = [NSColor(named: NSColor.Name("collectionview-background-color"))!]
        } else {
            collectionView.backgroundColors = [NSColor.init(white: 0.95, alpha: 1.0)]
        }
        super.init()
        collectionView.delegate = self
        collectionView.dataSource = self
        flowLayout.delegate = self
    }

    func process(_ timeEntries: [TimeEntryViewItem], showLoadMore: Bool) {

        // Catogrize into group by date
        let groups = timeEntries.groupSort(ascending: false, byDate: { $0.started })
        var sections = groups.compactMap { group -> TimeEntrySection? in
            guard let firstEntry = group.first else { return nil }
            let header = TimeEntryHeader(date: firstEntry.formattedDate, totalTime: firstEntry.duration)
            return TimeEntrySection(header: header, entries: group)
        }

        // Add load more cell if need
        if showLoadMore {
            sections.append(TimeEntrySection.loadMoreSection())
        }

        // Reload
        self.sections = sections
        if firstTime {
            firstTime = false
            DispatchQueue.main.asyncAfter(deadline: DispatchTime.now() + .seconds(1)) {
                self.collectionView.reloadData()
            }
        } else {
            collectionView.reloadData()
        }
    }
}

extension TimeEntryDatasource: NSCollectionViewDataSource, NSCollectionViewDelegate, NSCollectionViewDelegateFlowLayout {

    func numberOfSections(in collectionView: NSCollectionView) -> Int {
        return sections.count
    }

    func collectionView(_ collectionView: NSCollectionView,
                        numberOfItemsInSection section: Int) -> Int {
        let sectionItem = sections[section]
        return sectionItem.entries.count
    }

    func collectionView(_ collectionView: NSCollectionView,
                        itemForRepresentedObjectAt indexPath: IndexPath) -> NSCollectionViewItem {

        let section = sections[indexPath.section]

        if section.isLoadMore {
            return makeLoadMoreCell(with: collectionView, indexPath: indexPath)
        }

        return makeTimeEntryCell(with: collectionView, indexPath: indexPath)
    }

    func collectionView(_ collectionView: NSCollectionView,
                        viewForSupplementaryElementOfKind kind: NSCollectionView.SupplementaryElementKind,
                        at indexPath: IndexPath) -> NSView {
        let section = sections[indexPath.section]

        // Return empty view
        if section.isLoadMore {
            return NSView()
        }

        // Or normal
        guard let header = collectionView.makeSupplementaryView(ofKind: NSCollectionView.elementKindSectionHeader,
                                                          withIdentifier: NSUserInterfaceItemIdentifier("TimeHeaderView"),
                                                          for: indexPath) as? TimeHeaderView else {
                                                            fatalError("Can't cast to TimeHeaderView")
        }
        header.config(section.header)
        return header
    }

    func collectionView(_ collectionView: NSCollectionView,
                        layout collectionViewLayout: NSCollectionViewLayout,
                        referenceSizeForHeaderInSection section: Int) -> NSSize {
        let sectionData = sections[section]

        // We don't need header for load more cell
        // but we don't have choice to opt-out, so we return zero size
        if sectionData.isLoadMore {
            return CGSize.zero
        }

        // Actual size of header
        return headerSize
    }

    func collectionView(_ collectionView: NSCollectionView,
                        layout collectionViewLayout: NSCollectionViewLayout,
                        sizeForItemAt indexPath: IndexPath) -> NSSize {
        let section = sections[indexPath.section]
        if section.isLoadMore {
            return loaderMoreSize
        }
        return cellSize
    }

    private func makeTimeEntryCell(with collectionView: NSCollectionView,
                                  indexPath: IndexPath) -> NSCollectionViewItem {
        guard let cell = collectionView.makeItem(withIdentifier: NSUserInterfaceItemIdentifier("TimeEntryCell"),
                                                 for: indexPath) as? TimeEntryCell else {
                                                    fatalError()
        }
        let section = sections[indexPath.section]
        let item = section.entries[indexPath.item]

        // Render data
        cell.render(item)

        // Show / Hide the line
        let show = indexPath.item != (section.entries.count - 1)
        cell.showHorizontalLine(show)

        return cell
    }

    private func makeLoadMoreCell(with collectionView: NSCollectionView,
                                  indexPath: IndexPath) -> NSCollectionViewItem {
        guard let cell = collectionView.makeItem(withIdentifier: NSUserInterfaceItemIdentifier("LoadMoreCell"),
                                                 for: indexPath) as? LoadMoreCell else {
                                                    fatalError()
        }
        return cell
    }

    func collectionView(_ collectionView: NSCollectionView, didSelectItemsAt indexPaths: Set<IndexPath>) {
        guard let selectedIndex = collectionView.selectionIndexPaths.first,
            let timeEntryCell = collectionView.item(at: selectedIndex) as? TimeEntryCell else {
            return
        }

        if timeEntryCell.group {
            NotificationCenter.default.postNotificationOnMainThread(NSNotification.Name(kToggleGroup),
                                                                    object: timeEntryCell.groupName)
            collectionView.deselectItems(at: indexPaths)
            return
        }
        timeEntryCell.focusFieldName()
    }
}

extension TimeEntryDatasource: VertificalTimeEntryFlowLayoutDelegate {

    func isLoadMoreItem(at section: Int) -> Bool {
        let section = sections[section]
        return section.isLoadMore
    }
}
