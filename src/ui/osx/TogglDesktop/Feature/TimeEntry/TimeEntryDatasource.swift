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
    private(set) var isOpen = true

    init(header: TimeEntryHeader, entries: [TimeEntryViewItem], isLoadMore: Bool = false) {
        self.header = header
        self.entries = entries
        self.isLoadMore = isLoadMore
    }

    class func loadMoreSection() -> TimeEntrySection {
        let item = TimeEntryViewItem()
        item.loadMore = true
        return TimeEntrySection(header: TimeEntryHeader(date: "", totalTime: ""),
                                entries: [item],
                                isLoadMore: true)
    }

    func togglSection() {
        isOpen.toggle()
    }

}

@objc protocol TimeEntryDatasourceDraggingDelegate {

    func collectionView(_ collectionView: NSCollectionView, acceptDrop draggingInfo: NSDraggingInfo, indexPath: IndexPath, dropOperation: NSCollectionView.DropOperation) -> Bool

    func collectionView(_ collectionView: NSCollectionView, draggingSession session: NSDraggingSession, willBeginAt screenPoint: NSPoint, forItemsAt indexPaths: Set<IndexPath>)
}

@objcMembers
class TimeEntryDatasource: NSObject {

    struct Constants {
        static let TimeEntryCellNibName = NSNib.Name("TimeEntryCell")
        static let TimeEntryCellID = NSUserInterfaceItemIdentifier("TimeEntryCell")
        static let LoadMoreCellNibName = NSNib.Name("LoadMoreCell")
        static let LoadMoreCellID = NSUserInterfaceItemIdentifier("LoadMoreCell")
        static let TimeHeaderNibName = NSNib.Name("TimeHeaderView")
        static let TimeHeaderID = NSUserInterfaceItemIdentifier("TimeHeaderView")
        static let DecoratorViewNibName = NSNib.Name("TimeDecoratorView")
        static let DecoratorViewKind = "TimeDecoratorView"
    }

    // MARK: Variable
    weak var delegate: TimeEntryDatasourceDraggingDelegate?
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

    var count: Int {
        return sections.count
    }
    
    // MARK: Init

    init(collectionView: NSCollectionView) {
        self.sections = []
        self.collectionView = collectionView
        super.init()
        collectionView.delegate = self
        collectionView.dataSource = self
        registerAllCells()
        initCommon()
    }

    // MARK: Public

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
        reload(with: sections)
    }

    @objc func object(at indexPath: IndexPath) -> TimeEntryViewItem? {
        guard let section = sections[safe: indexPath.section] else { return nil }
        return section.entries[safe: indexPath.item]
    }

    @objc func indexPath(for viewItem: TimeEntryViewItem) -> IndexPath? {
        for (sectionIndex, section) in sections.enumerated() {
            for (rowIndex, entry) in section.entries.enumerated() {
                if entry.guid == viewItem.guid {
                    return IndexPath(item: rowIndex, section: sectionIndex)
                }
            }
        }
        return nil
    }

    @objc func object(with guid: String) -> TimeEntryViewItem? {
        for section in sections {
            for entry in section.entries {
                if entry.guid == guid {
                    return entry
                }
            }
        }
        return nil
    }

    private func reload(with sections: [TimeEntrySection]) {
        // Reload
        self.sections = sections
        collectionView.reloadData()
    }
}

// MARK: Private

extension TimeEntryDatasource {

    fileprivate func initCommon() {

        // Background
        collectionView.wantsLayer = true
        if #available(OSX 10.13, *) {
            collectionView.backgroundColors = [NSColor(named: NSColor.Name("collectionview-background-color"))!]
        } else {
            collectionView.backgroundColors = [NSColor.init(white: 0.95, alpha: 1.0)]
        }

        // Flow
        let flowLayout = VertificalTimeEntryFlowLayout()
        flowLayout.delegate = self
        collectionView.collectionViewLayout = flowLayout

        // Dragging
        collectionView.registerForDraggedTypes([NSPasteboard.PasteboardType.string])
        collectionView.setDraggingSourceOperationMask(NSDragOperation.move, forLocal: true)
        collectionView.setDraggingSourceOperationMask(NSDragOperation.link, forLocal: false)
    }

    fileprivate func registerAllCells() {
        collectionView.register(NSNib(nibNamed: Constants.TimeEntryCellNibName, bundle: nil), forItemWithIdentifier: Constants.TimeEntryCellID)
        collectionView.register(NSNib(nibNamed: Constants.LoadMoreCellNibName, bundle: nil), forItemWithIdentifier: Constants.LoadMoreCellID)
        collectionView.register(NSNib(nibNamed: Constants.TimeHeaderNibName, bundle: nil),
                                forSupplementaryViewOfKind:NSCollectionView.elementKindSectionHeader,
                                withIdentifier: Constants.TimeHeaderID)
    }
}

// MARK: NSCollectionViewDataSource

extension TimeEntryDatasource: NSCollectionViewDataSource, NSCollectionViewDelegate, NSCollectionViewDelegateFlowLayout {

    func numberOfSections(in collectionView: NSCollectionView) -> Int {
        return sections.count
    }

    func collectionView(_ collectionView: NSCollectionView,
                        numberOfItemsInSection section: Int) -> Int {
        let sectionItem = sections[section]
        if sectionItem.isOpen {
            return sectionItem.entries.count
        }
        return 0
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
        header.config(section.header,
                      section: indexPath.section)
        header.delegate = self
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
        if indexPath.item == section.entries.count - 1 {
            cell.applyMaskForBottomCorner()
        }

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
}

// MARK: Dragging

extension TimeEntryDatasource {

    func collectionView(_ collectionView: NSCollectionView,
                        acceptDrop draggingInfo: NSDraggingInfo,
                        index: Int,
                        dropOperation: NSCollectionView.DropOperation) -> Bool {
        return true
    }

    func collectionView(_ collectionView: NSCollectionView,
                        validateDrop draggingInfo: NSDraggingInfo,
                        proposedIndexPath proposedDropIndexPath: AutoreleasingUnsafeMutablePointer<NSIndexPath>,
                        dropOperation proposedDropOperation: UnsafeMutablePointer<NSCollectionView.DropOperation>) -> NSDragOperation {
         return .move
    }

    func collectionView(_ collectionView: NSCollectionView, acceptDrop draggingInfo: NSDraggingInfo, indexPath: IndexPath, dropOperation: NSCollectionView.DropOperation) -> Bool {
        guard let delegate = delegate else { return
            false
        }
        let result =  delegate.collectionView(collectionView,
                                acceptDrop: draggingInfo,
                                indexPath: indexPath,
                                dropOperation: dropOperation)
        return result
    }

    func collectionView(_ collectionView: NSCollectionView, draggingSession session: NSDraggingSession, willBeginAt screenPoint: NSPoint, forItemsAt indexPaths: Set<IndexPath>) {
        guard let delegate = delegate else {
            return
        }
        delegate.collectionView(collectionView,
                                draggingSession: session,
                                willBeginAt: screenPoint,
                                forItemsAt: indexPaths)
    }

    func collectionView(_ collectionView: NSCollectionView, canDragItemsAt indexPaths: Set<IndexPath>, with event: NSEvent) -> Bool {
        return true
    }

    func collectionView(_ collectionView: NSCollectionView, pasteboardWriterForItemAt indexPath: IndexPath) -> NSPasteboardWriting? {
        guard let item = object(at: indexPath) else { return nil }
        guard !item.loadMore else { return nil }

        // Save indexpath
        let data = NSKeyedArchiver.archivedData(withRootObject: indexPath)
        let pbItem = NSPasteboardItem()
        pbItem.setData(data, forType: NSPasteboard.PasteboardType.string)
        return pbItem
    }
}

// MARK: VertificalTimeEntryFlowLayoutDelegate

extension TimeEntryDatasource: VertificalTimeEntryFlowLayoutDelegate {

    func isLoadMoreItem(at section: Int) -> Bool {
        let section = sections[section]
        return section.isLoadMore
    }
}

// MARK: TimeHeaderViewDelegate

extension TimeEntryDatasource: TimeHeaderViewDelegate {

    func togglSection(at section: Int) {
        print("Toggl section \(section)")
        let section = sections[section]
        section.togglSection()
        collectionView.reloadData()
    }
}
