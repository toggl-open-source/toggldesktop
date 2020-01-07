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
        let item = TimeEntryViewItem()
        item.loadMore = true
        return TimeEntrySection(header: TimeEntryHeader(date: "", totalTime: ""),
                                entries: [item],
                                isLoadMore: true)
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
    private var sections: [TimeEntrySection]
    private var currentIndexPath = IndexPath(item: 0, section: 0)
    private let collectionView: TimeEntryCollectionView
    private let queue = DispatchQueue(label: "com.toggl.toggldesktop.TogglDesktop.timeentryqueue")
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
    
    private(set) var isShowLoadMore = false

    // MARK: Init

    init(collectionView: TimeEntryCollectionView) {
        self.sections = []
        self.collectionView = collectionView
        super.init()
        collectionView.delegate = self
        collectionView.dataSource = self
        registerAllCells()
        initCommon()
    }
    
    // MARK: Public

    @objc func loadMoreTimeEntryIfNeed(at date: Date) {
        guard isShowLoadMore,
            let lastDate = lastDateInSections() else { return }

        // If the date of timeline is before the last day and the Load More btn is showing
        // We should load more item
        if date < lastDate {
            DesktopLibraryBridge.shared().loadMoreTimeEntry()
        }
    }

    func selectFirstItem() {
        // Check if there is a least 1 item
        guard sections.first?.entries.first != nil else { return }
        collectionView.window?.makeFirstResponder(collectionView)

        // Deselect all because TE is multiple-selections
        collectionView.deselectAll(self)
        collectionView.selectItems(at: Set<IndexPath>(arrayLiteral: IndexPath(item: 0, section: 0)),
                                   scrollPosition: .left)
    }

    func process(_ timeEntries: [TimeEntryViewItem], showLoadMore: Bool) {
        isShowLoadMore = showLoadMore

        // Catogrize into group by date
        let groups = timeEntries.groupSort(ascending: false, byDate: { $0.started })
        var sections = groups.compactMap { group -> TimeEntrySection? in
            guard let firstEntry = group.first else { return nil }
            let header = TimeEntryHeader(date: firstEntry.formattedDate, totalTime: firstEntry.dateDuration)
            return TimeEntrySection(header: header, entries: group)
        }

        // Add load more cell if need
        if showLoadMore {
            sections.append(TimeEntrySection.loadMoreSection())
        }

        // Touch bar
        if #available(OSX 10.12.2, *) {
            TouchBarService.shared.updateTimeEntryList(timeEntries)
        }

        // Reload
        reload(with: sections)
    }

    @objc func object(at indexPath: IndexPath) -> TimeEntryViewItem? {
        return queue.sync(flags: .barrier) {
            guard let section = sections[safe: indexPath.section] else { return nil }
            return section.entries[safe: indexPath.item]
        }
    }

    @objc func indexPath(for viewItem: TimeEntryViewItem) -> IndexPath? {
        return queue.sync(flags: .barrier) {
            for (sectionIndex, section) in sections.enumerated() {
                for (rowIndex, entry) in section.entries.enumerated() {
                    if entry.guid == viewItem.guid {
                        return IndexPath(item: rowIndex, section: sectionIndex)
                    }
                }
            }
            return nil
        }
    }

    @objc func object(with guid: String) -> TimeEntryViewItem? {
        return queue.sync(flags: .barrier) {
            for section in sections {
                for entry in section.entries {
                    if entry.guid == guid {
                        return entry
                    }
                }
            }
            return nil
        }
    }

    @objc func previousIndexPath(from indexPath: IndexPath) -> IndexPath? {

        // It means previous row must be last cell in previous section
        if indexPath.item == 0 {
            let previousSectionIndex = indexPath.section - 1
            guard previousSectionIndex >= 0 && previousSectionIndex < self.count else {
                return nil
            }
            guard let previousSection = sectionItem(at: previousSectionIndex) else {
                return nil
            }
            let lastRowIndex = previousSection.entries.count - 1
            return IndexPath(item: lastRowIndex, section: previousSectionIndex)
        }

        return IndexPath(item: indexPath.item - 1, section: indexPath.section)
    }

    private func reload(with sections: [TimeEntrySection]) {

        // Find the GUID of selection cell
        var guid: String?
        if let lastSelection = Array(collectionView.selectionIndexPaths).last,
            let cell = collectionView.item(at: lastSelection) as? TimeEntryCell {
            guid = cell.guid
        }

        self.sections.removeAll()
        self.sections.append(contentsOf: sections)
        collectionView.reloadData()

        // Reselect cell with no animation
        if let guid = guid {
            reselectTimeEntryRow(with: guid)
        }

        //TODO: Refactor the hack code
        // Only happen If we enable "Show ScrollBar" is "Always" or "Auto"
        // The scroller bar appear and make the size of collection view is smaller
        // -> We have to re-calulate the layout of DecoratorView again
        // After 0.1s -> because the ScrollBar appear later after collectionView.reloadData is called
        DispatchQueue.main.asyncAfter(deadline: DispatchTime.now() + .microseconds(100)) {
            self.collectionView.collectionViewLayout?.invalidateLayout()
        }
    }

    private func reselectTimeEntryRow(with guid: String) {

        // Prevent jump
        DispatchQueue.main.async {

            // Find the cell with samw GUID
            let cell = self.collectionView.visibleItems().first(where:{ (item) in
                if let cell = item as? TimeEntryCell {
                    if cell.guid == guid {
                        return true
                    }
                }
                return false
            })

            // Select if the cell is available
            // Prevent invalid selection if we delete cell
            if let cell = cell,
                let index = self.collectionView.indexPath(for: cell) {
                self.collectionView.selectItems(at:  Set<IndexPath>(arrayLiteral: index),
                                                scrollPosition: [])
            }
        }
    }

    fileprivate func sectionItem(at section: Int) -> TimeEntrySection? {
        return queue.sync(flags: .barrier) {
            return sections[safe: section]
        }
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

    fileprivate func lastDateInSections() -> Date? {
        let lastSection = sections.reversed().first { !$0.isLoadMore }
        return lastSection?.entries.last?.started
    }
}

// MARK: NSCollectionViewDataSource

extension TimeEntryDatasource: NSCollectionViewDataSource, NSCollectionViewDelegate, NSCollectionViewDelegateFlowLayout {
    func numberOfSections(in collectionView: NSCollectionView) -> Int {
        return sections.count
    }

    func collectionView(_ collectionView: NSCollectionView,
                        numberOfItemsInSection section: Int) -> Int {
        let sectionItem = self.sectionItem(at: section)
        return sectionItem?.entries.count ?? 0
    }

    func collectionView(_ collectionView: NSCollectionView,
                        itemForRepresentedObjectAt indexPath: IndexPath) -> NSCollectionViewItem {

        guard let section = sectionItem(at: indexPath.section) else { return NSCollectionViewItem() }

        if section.isLoadMore {
            return makeLoadMoreCell(with: collectionView, indexPath: indexPath)
        }

        return makeTimeEntryCell(with: collectionView, indexPath: indexPath)
    }

    func collectionView(_ collectionView: NSCollectionView,
                        viewForSupplementaryElementOfKind kind: NSCollectionView.SupplementaryElementKind,
                        at indexPath: IndexPath) -> NSView {
        guard let section = sectionItem(at: indexPath.section) else { return NSView() }

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
        guard let sectionData = sectionItem(at: section) else { return .zero }

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
        guard let section = sectionItem(at: indexPath.section) else { return .zero }
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
        guard let section = sectionItem(at: indexPath.section) else { return NSCollectionViewItem() }
        if let item = section.entries[safe: indexPath.item] {
            cell.render(item)
        }
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
        let data = NSKeyedArchiver.archivedData(withRootObject: Array(collectionView.selectionIndexPaths))
        let pbItem = NSPasteboardItem()
        pbItem.setData(data, forType: NSPasteboard.PasteboardType.string)
        return pbItem
    }
}

// MARK: VertificalTimeEntryFlowLayoutDelegate

extension TimeEntryDatasource: VertificalTimeEntryFlowLayoutDelegate {

    func isLoadMoreItem(at section: Int) -> Bool {
        guard let section = sectionItem(at: section) else { return false }
        return section.isLoadMore
    }
}

// MARK: TimeHeaderViewDelegate

extension TimeEntryDatasource: TimeHeaderViewDelegate {

    func togglSection(at section: Int) {
        // For expanding group logic in future
    }
}
